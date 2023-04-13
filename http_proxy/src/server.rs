#[derive(Debug)]
struct Server {
    listener: TcpListener,
    queue: std::sync::mpsc::SyncSender<TcpStream>,
    stop_flag: Arc<std::sync::atomic::AtomicBool>,
    thread_handles: Vec<std::thread::JoinHandle<()>>,
}

impl Server {
    fn new(port: u16) -> Result<Self, ()> {
        let listener = TcpListener::bind(format!("127.0.0.1:{}", port)).unwrap();
    
        let n = std::thread::available_parallelism().unwrap_or(NonZeroUsize::new(4).unwrap()).into();
        let mut threads = Vec::with_capacity(n);

        let stop_flag = Arc::new(AtomicBool::new(false));

        let (tx, rx) = sync_channel::<TcpStream>(1024);

        let rx = Arc::new(Mutex::new(rx));

        for _ in 0..n {
            let stop_flag_cpy = Arc::clone(&stop_flag);
            let rx_copy = Arc::clone(&rx);

            let hanlde = std::thread::spawn(move || loop {
                if stop_flag_cpy.load(std::sync::atomic::Ordering::Relaxed) {
                    break;
                }

                let in_conn = rx_copy.lock().unwrap().recv().unwrap();
                
                // in_conn.re

                // read data from
            });

            threads.push(handle);
        }


        Ok(Self {
            listener, 
            queue: tx,
            stop_flag,
            thread_handles: threads
        })
    }

    fn run(&self) {
        // register callback


        for stream in self.listener.incoming() {
            match stream {
                Ok(stream) => self.queue.send(stream).unwrap(),
                Err(_) => panic!("Listener socket closed")
            };
        }
    }
}

