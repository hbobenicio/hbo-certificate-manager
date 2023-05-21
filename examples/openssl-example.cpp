#include <iostream>
#include <vector>
#include <filesystem>
#include <cassert>

#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/window.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/button.h>
#include <gtkmm/filechooser.h>
#include <gtkmm/filechoosernative.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

using std::string;
using std::stringstream;
using std::vector;
using std::cout;
using std::cerr;

Glib::RefPtr<Gtk::Application> app;

// for (auto cert: server_cert_chain) {
//     X509_NAME* subject_name = X509_get_subject_name(cert);
//     X509_NAME_print_ex_fp(stderr, subject_name, 2, 0);
//     cerr << '\n';
// }

namespace openssl
{
    void init()
    {
        SSL_load_error_strings();
        ERR_load_BIO_strings();
        OpenSSL_add_all_algorithms();
    }

    vector<X509*> convert_cert_stack_into_vector(STACK_OF(X509)* certs)
    {
        vector<X509*> certs_vector(sk_X509_num(certs));

        for (int i = 0; i < sk_X509_num(certs); i++) {
            X509* cert = sk_X509_value(certs, i);

            X509_up_ref(cert);

            certs_vector[i] = cert;
        }

        return certs_vector;
    }

    void print_certs(const vector<X509*>& certs)
    {
        BIO* stdout_bio = BIO_new_fp(stdout, BIO_NOCLOSE);
        assert(stdout_bio != NULL);

        for (X509* cert: certs) {
            PEM_write_bio_X509(stdout_bio, cert);
        }
    }

    void save_certs(const vector<X509*>& certs, const string& output_file_path)
    {
        BIO* output_file_bio = BIO_new_file(output_file_path.c_str(), "wb");
        assert(output_file_bio != nullptr);

        for (X509* cert: certs) {
            PEM_write_bio_X509(output_file_bio, cert);
        }
        
        BIO_free(output_file_bio);
    }
    
    class TlsConnection {
    private:
        static constexpr long TimeoutInSeconds = 5;

        SSL_CTX* ctx = nullptr;
        BIO* bio = nullptr;
        SSL* ssl = nullptr;

    public:
        TlsConnection(const string& address, const string& hostname)
        {
            cerr << "info: connecting... address=\"" << address << "\"\n";

            this->ctx = SSL_CTX_new(TLS_client_method());
            assert(this->ctx != nullptr && "OpenSSL Context must be created");

            assert(SSL_CTX_set_default_verify_paths(this->ctx) == 1);
            SSL_CTX_set_timeout(this->ctx, TlsConnection::TimeoutInSeconds);

            this->bio = BIO_new_ssl_connect(ctx);
            assert(this->bio != nullptr && "TlsConnection BIO must be created");

            BIO_get_ssl(this->bio, &this->ssl);
            assert(this->ssl != nullptr && "SSL Session must be created");

            SSL_set_mode(this->ssl, SSL_MODE_AUTO_RETRY);
            assert(SSL_set_tlsext_host_name(this->ssl, hostname.c_str()) == 1);

            //NOTE this may include the port too
            //TODO Why this doesn't work with www.google.com?
            BIO_set_conn_hostname(this->bio, address.c_str());
        }

        ~TlsConnection()
        {
            if (this->bio) {
                cerr << "debug: connection: freeing BIO...\n";
                BIO_free_all(this->bio);
            }
            if (this->ctx) {
                cerr << "debug: connection: freeing openssl context...\n";
                SSL_CTX_free(this->ctx);
            }
        }

        void connect()
        {
            //TODO as this is blocking, start it in a new thread (do not block the UI Main Thread)
            //TODO improve error handling (connection may fail. do not continue then...)
            if (BIO_do_connect(this->bio) != 1) {
                cerr << "error: connection failed: openssl reason: " << ERR_reason_error_string(ERR_get_error()) << '\n';
                return;
            }

            cerr << "debug: connected.\n";

            if (SSL_get_verify_result(this->ssl) != X509_V_OK) {
                cerr << "error: openssl verification failed. openssl reason: " << ERR_reason_error_string(ERR_get_error()) << '\n';
                return;
            }

            cerr << "debug: openssl verification succeeded.\n";
        }

        vector<X509*> get_peer_cert_chain() const
        {
            // NOTES
            // If the session is resumed peers do not send certificates so a NULL pointer is returned by these functions. Applications can call SSL_session_reused() to determine whether a
            // session is resumed.
            // The reference count of each certificate in the returned STACK_OF(X509) object is not incremented and the returned stack may be invalidated by renegotiation.  If applications
            // wish to use any certificates in the returned chain indefinitely they must increase the reference counts using X509_up_ref() or obtain a copy of the whole chain with
            // X509_chain_up_ref()
            STACK_OF(X509)* cert_chain = SSL_get_peer_cert_chain(this->ssl);
            // STACK_OF(X509)* verified_cert_chain = SSL_get0_verified_chain(this->ssl);
            assert(cert_chain != nullptr);
            // assert(verified_cert_chain != nullptr);
            // cert_chain = X509_chain_up_ref(cert_chain);
            // verified_cert_chain = X509_chain_up_ref(verified_cert_chain);
            //TODO call X509_chain_down_ref?

            return convert_cert_stack_into_vector(cert_chain);
        }
    };
}

class MainApplicationWindow: public Gtk::ApplicationWindow {
private:
    Gtk::HeaderBar header_bar;
    Gtk::Label something{"something"};
    
public:
    MainApplicationWindow()
    {
        ui_setup();
    }

    void ui_setup()
    {
        set_default_size(800, 125);
        
        header_bar.set_decoration_layout("icon:close");
        header_bar.add(something);

        this->set_titlebar(header_bar);

        show_all_children();
    }
};

class MainWindow: public Gtk::Window {
private:
    Gtk::Box vbox{Gtk::ORIENTATION_VERTICAL};

    Gtk::Box address_box{Gtk::ORIENTATION_HORIZONTAL};
    Gtk::Label host_label{"Host:"};
    Gtk::Entry host_entry;
    Gtk::Label port_label{"Port:"};
    Gtk::SpinButton port_spin_button;

    Gtk::Button connect_button{"Connect"};

    Glib::RefPtr<Gtk::FileChooserNative> file_chooser = Gtk::FileChooserNative::create("Choose a file", *this, Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SAVE);
    vector<X509*> server_cert_chain;

public:
    MainWindow()
    {
        ui_setup();
    }

    ~MainWindow() override
    {
        clean_server_cert_chain();
    }

private:

    void clean_server_cert_chain()
    {
        for (X509* cert: server_cert_chain) {
            X509_free(cert);
        }
        this->server_cert_chain.clear();
    }

    void ui_setup()
    {
        set_title("Chaveiro");
        set_default_size(800, 125);

        this->vbox.set_margin_left(10);
        this->vbox.set_margin_right(10);
        this->vbox.set_margin_top(10);
        this->vbox.set_margin_bottom(10);
        add(this->vbox);
        this->vbox.pack_start(this->address_box, Gtk::PACK_SHRINK);

        this->connect_button.set_halign(Gtk::Align::ALIGN_END);
        this->vbox.pack_end(connect_button, false, false, 10);

        this->address_box.set_margin_top(10);
        this->address_box.pack_start(this->host_label, false, false, 0);
        this->address_box.pack_start(this->host_entry, true, true, 10);
        this->address_box.pack_start(this->port_label, false, false, 10);
        this->address_box.pack_start(this->port_spin_button, false, false, 0);

        this->port_spin_button.set_numeric(true);
        {
            constexpr double value = 443.0;
            constexpr double lower = 1.0;
            const double upper = std::pow(2.0, 16.0);
            this->port_spin_button.set_adjustment(Gtk::Adjustment::create(value, lower, upper));
        }

        this->file_chooser->set_select_multiple(false);

        // Events.
        // We override the default event signal handler.
        add_events(Gdk::KEY_PRESS_MASK);

        this->connect_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_connect_button_clicked));

        this->file_chooser->signal_response().connect(sigc::mem_fun(*this, &MainWindow::on_file_chooser_response));

        show_all_children();
    }

    void on_connect_button_clicked()
    {
        connect();
    }

    // Override default signal handler
    // returning true, cancels the propagation of the event
    // if the event has not been handled, call the base class
    bool on_key_press_event(GdkEventKey* key_event) override
    {
        switch (key_event->keyval) {
        case GDK_KEY_Escape:
            app->quit();
            return true;
        
        case GDK_KEY_Return:
            connect();
            return true;
        
        default:
            return Gtk::Window::on_key_press_event(key_event);
        }
    }

    void on_file_chooser_response(int response_id)
    {
        (void) response_id;

        string output_file_path = this->file_chooser->get_file()->get_path();
        openssl::save_certs(this->server_cert_chain, output_file_path);
    }

    void connect()
    {
        Glib::ustring hostname = this->host_entry.get_text();
        int port = this->port_spin_button.get_value_as_int();

        stringstream ss;
        ss << hostname;
        ss << ':';
        ss << port;
        string address = ss.str();

        openssl::TlsConnection connection{address, hostname.raw()};
        connection.connect();

        clean_server_cert_chain();
        this->server_cert_chain = connection.get_peer_cert_chain();
        cerr << "info: got " << server_cert_chain.size() << " certificate(s) from the server\n";

        this->file_chooser->show();
    }
};

int main(int argc, char** argv)
{
    openssl::init();

    app = Gtk::Application::create(argc, argv, "br.com.hugobenicio.chaveiro");
    // MainWindow main_window;
    // return app->run(main_window);
    MainApplicationWindow main_app_window;
    return app->run(main_app_window);
}
