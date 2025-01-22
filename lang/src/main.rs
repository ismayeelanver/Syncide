use std::process::exit;
mod parser;
use parser::lexer;
use parser::error::CompilerError;

fn main() {
    if let Err(e) = run() {
        eprintln!("Compilation failed. Exited at code: 1");
        exit(1);
    }
}



fn run() -> Result<(), CompilerError> {
    let mut lexer = lexer::Lexer::new(&"examples/test.sr").unwrap_or_else(|_| {
        eprintln!("Error: Unable to open file.");
        exit(1);
    });

    let tokens = lexer.tokenize();
    let ast = parser::Parser::new(tokens.clone(), lexer.filename).parse()?;
    
    // Do something with the AST here
    println!("Successfully parsed AST: {:#?}", ast);
    
    Ok(())
}
