use std::process::exit;

mod parser;

use parser::lexer;


fn main() {
    let mut lexer: lexer::Lexer = lexer::Lexer::new(&"examples/test.sy").unwrap_or_else(|_| {
        eprintln!("Error: Unable to open file.");
        exit(1);
    });

    let tokens = lexer.tokenize();

    let mut parser = parser::Parser::new(tokens.clone(), lexer.filename);

    let ast = parser.parse();



    println!("{:#?}", ast);

}
