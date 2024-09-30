use clap::Parser;
use std::net::SocketAddr;
use std::sync::atomic::AtomicBool;
use std::sync::Arc;
use tracing::info;
use warthog_lib::Options;
use warthog_wow::ApplicationOptions;

#[derive(clap::Parser, Debug)]
#[command(version, about)]
struct Args {
    /// Address to host auth server on.
    #[arg(short, long, default_value = "0.0.0.0:3724")]
    address: SocketAddr,
    /// Randomize PIN grid number locations.
    #[arg(short, long, default_value = "false")]
    pin_grid_randomize: bool,
    /// Address to reply to inter server communication on.
    #[arg(short, long, default_value = "0.0.0.0:8086")]
    reply_address: SocketAddr,
}

impl Args {
    fn to_options(self) -> (Options, ApplicationOptions) {
        (
            Options {
                address: self.address,
                randomize_pin_grid: self.pin_grid_randomize,
                max_concurrent_users: 1000,
            },
            ApplicationOptions {
                reply_address: self.reply_address,
                use_pin: false,
                use_matrix_card: false,
            },
        )
    }
}

#[tokio::main]
async fn main() {
    tracing_subscriber::fmt::init();
    let args = Args::parse();

    let (options, application_options) = args.to_options();
    info!(?options, ?application_options, "options parsed");
    let should_run = Arc::new(AtomicBool::new(true));

    warthog_wow::lib_main(options, application_options, should_run).await;
}
