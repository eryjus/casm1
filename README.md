# Century Assembler (1-pass version)

I am working from the Assemblers and Loaders manual available on the internet (copy available [here](http://www.davidsalomon.name/assem.advertis/asl.pdf)).  In this there are several exercises that work toward building a functioning assembler (well, actually 2 functioning assemblers -- a 1-pass assembler and a 2-pass assembler),

This project focuses in on the 1-pass assembler.

There will be several iterations of this project as we develop it out following this guide.  Ultimately, I do not expect that this assembler will go very far.

As a result (based on the book's assignment) I will be starting with M (Memory size) of 1024 (requiring 10 bits to address) and N (Word size) of 16 (or a 16-bit word).  This, then, will leave 6 bits for the opcode, which is more than enough, and 10 bits for the operand.  This configuration is not very useful for many architectures.

