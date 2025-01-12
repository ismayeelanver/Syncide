use std::process::exit;

mod parser;

use parser::lexer;


fn main() {
    let mut lexer: lexer::Lexer = lexer::Lexer::new(&"examples/test.sy").unwrap_or_else(|_| {
        eprintln!("Error: Unable to open file.");
        exit(1);
    });


    eprintln!("[Lexical Analysis]\texamples/test.sy\n\n");

    let tokens = lexer.tokenize();

    eprintln!("\t[Success]\tSuccess Lexing the source code into tokens!\n\n");

    eprintln!("[Parser]\texamples/test.sy\n\n");

    let ast = parser::Parser::new(tokens.clone(), lexer.filename).parse();

    eprintln!("\t[Success]\tParsing the code into the tree! here's a great look into it:\n\n{:#?}", ast);


}
