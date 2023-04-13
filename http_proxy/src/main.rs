use std::io::prelude::*;
use std::net::{TcpStream, TcpListener, SocketAddr};
use std::collections::HashMap;
use std::num::NonZeroUsize;
use std::str::FromStr;
use std::sync::{Arc, Mutex};
use std::sync::atomic::AtomicBool;
use std::sync::mpsc::sync_channel;

use regex::Regex;


mod message;
mod server;

fn main() {
    let args: Vec<String> = std::env::args().collect();

    if args.len() < 2 {
        println!("Not enough arguments.");
        println!("Usage: proxy <port>");
        return;
    }

    let port = args.get(1).unwrap().parse::<u16>().expect("Invalid port number");
    let listener = TcpListener::bind(format!("localhost:{}", port)).expect("Could not bind to port");

    for stream in listener.incoming() {
        if stream.is_err() {
            println!("Invalid stream");
            break;
        } 

        let stream = stream.unwrap();
        let mut req: String;
        let n = stream.read_to_string(&mut req);

        let regex = Regex::new(r"http://").unwrap();
        
        // drop(stream);
    }

    drop(listener);
}
