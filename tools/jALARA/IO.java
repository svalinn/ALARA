import java.io.*;

public class IO {

    // **********************************************************************
    // openInputFile
    //
    // Open the file with the given name for reading.
    // Throw a FileNotFoundException if the file does not exist.
    // **********************************************************************
    public static StreamTokenizer openInputFile(String fileName)
	throws FileNotFoundException {
	File inFile = new File(fileName);
	if (!inFile.exists()) {
	    throw new FileNotFoundException();
	}
	return (new StreamTokenizer(new BufferedReader(
						    new FileReader(inFile))));
    }

    // **********************************************************************
    // openOutputFile
    //
    // Open the file with the given name for writing.
    // Throw a FileNotFoundException if the file does not exist.
    // **********************************************************************
    public static PrintWriter openOutputFile(String fileName) 
    throws IOException {
	File outFile = new File(fileName);
       	return (new PrintWriter(new BufferedWriter(new FileWriter(outFile))));
    }

    // **********************************************************************
    // readWord
    //
    // Read and return the next word from the file associated with the given
    // StreamTokenizer.  Return null if at end-of-file.
    //
    // Throw an IOException if there is a problem reading the file.
    // **********************************************************************
    public static String readWord(StreamTokenizer input) throws IOException {
	if (input.nextToken() != StreamTokenizer.TT_EOF) {
	    //if (input.ttype != StreamTokenizer.TT_WORD) {
		// bad data in input -- throw exception with error msg
		//throw new IOException("Bad input on line " + input.lineno());
	    //}
	    return(input.sval);
	}
	return null;
    }


    // **********************************************************************
    // promptString
    //
    // use toString() to output the prompt message passed in
    // then gets an input from the user 
    // then return the string
    //
    // **********************************************************************
    public static String promptString(String message) {
	BufferedReader stdin = new BufferedReader(new InputStreamReader(System.in));
	System.out.print(message);
	String s = null;		
	try{
	    s = stdin.readLine();
	}catch(IOException ex){
	    System.err.println(ex + "in IO.promptString()");
	}
	
	return s;
    }

    // **********************************************************************
    // promptInt
    //
    // use toString() to output the prompt message passed in
    // then gets an input from the user
    // cast it into int if fails output warnning and call promptInt again
    // then return the int
    //
    // **********************************************************************
    public static int promptInt(String message) {
        BufferedReader stdin = new BufferedReader(new InputStreamReader(System.in));
	System.out.print(message);
	String s = null;
	int i;

        try{
            s = stdin.readLine();
        }catch(IOException ex){
            System.err.println(ex + "in IO.promptString()");
        }

	try{
	    i = Integer.parseInt(s);
	}
	catch(NumberFormatException ex){
	    System.err.println("\nPlease enter an integer.");
	    i = promptInt(message);
	}
        return i;
    }

    // **********************************************************************
    // promptMenuOption
    //
    // pass: message --> the message to put outputed
    //       numOption --> the number of options available
    // return: the option uer selects
    //
    // **********************************************************************
    public static int promptMenuOption(String message, int numOption) {
        BufferedReader stdin = new BufferedReader(new InputStreamReader(System.in));
        System.out.print(message);
        String s = null;
        int i;

        try{
            s = stdin.readLine();
	    if( s.compareTo("startusingpowermode") == 0 )
		return 0;
	    if( s.compareTo("quitusingpowermode") == 0)
                return -1;
        }catch(IOException ex){
            System.err.println(ex + "in IO.promptString()");
        }

        try{
            i = Integer.parseInt(s);
	    if( i <= 0 || i > numOption ){
		System.out.println("\nPlease choose from 1 to " + numOption );
		i = promptMenuOption(message, numOption);
	    }
        }
        catch(NumberFormatException ex){
            System.err.println("\nPlease enter an integer.");
            i = promptMenuOption(message, numOption);
        }
        return i;
    }

}
