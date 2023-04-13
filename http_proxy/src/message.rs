use std::str::FromStr;
use std::collections::HashMap;

#[derive(Debug, Clone, PartialEq)]
enum StatusLine {
    Request{resource_path: String, request_type: String, http_version: String},
    Response{status: String}
}

#[derive(Debug, Clone)]
struct HttpMessage {
    http_version: String,
    status_line: StatusLine,
    header: HashMap<String, String>,
    body: Option<String>
}

#[derive(Debug, PartialEq, Eq)]
struct ParseHttpMessageError;

// impl FromStr for HttpMessage {
//     type Err = ParseHttpMessageError;
//
//     fn from_str(s: &str) -> Result<Self, Self::Err> {
//         // Err()
//     }
// }

impl ToString for HttpMessage {

    fn to_string(&self) -> String {
       let mut s = String::new(); 

       for (k, v) in self.header.iter() {
       }
        
       s
    }

}
