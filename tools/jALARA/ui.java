
import java.sql.*;
import java.io.*;

class ui{
    private String username = null;
    private String password = null;
    private String server = null;
    private String database = null;
    private boolean powermode = false;
    private Connection conn;
    
    private InOut inout; //handle business between server and the software

    //*******************************************************************
    //Use: constructor
    //Pass: none
    //Return: None
    //******************************************************************
    public ui(){
	inout = new InOut();
	conn = null;
    }
    
    public void launch(){
	//oepn configure file
	try{
	    //Load Configure File
	    StreamTokenizer inFile = IO.openInputFile("configure");
	    username = IO.readWord(inFile);
	    server = IO.readWord(inFile);
	    database = IO.readWord(inFile);
	    //System.out.println("Username; " + username);
	    //System.out.println("Server: " + server);
	    //System.out.println("Database: " + database);
 
	    System.out.println("Welcome to jALARA\n\n");
	    login();
	    mainMenu();
	    System.out.println("jALARA Has Quit");
	
	}catch(IOException ex){
	    System.out.println("** configure file not found **");
	}//end try catch
	
    }
    
    private void login(){
	password = IO.promptString("Please Enter The Password: ");
	System.out.println("Connect to Server : " + server +
			   "\nUsername: "+ username + "\n");
	conn = inout.connectDBServer(server, database, username, password, conn);
	if(conn == null) {
	    System.out.println("Please Check Your Password. \nIf you think the password is correct,"
			       +"make sure your configurations are correct in configure file \n");
	    login();
	}
    }
    
    private void mainMenu(){
	boolean b = true;
	while(b){
	    clr();
	    //###############################################################
            //CHANGE THE FOLLOWING CASE VALUE WHEN MENU IS EDITED
            //###############################################################
	    int option = 3;
	    if( powermode ) {
		option = 4;
		System.out.println(" ** POWERMODE ** ");
	    }
	    System.out.println("Please Choose From 1 to " + option);
	    System.out.println("(1)Read Binary File");
	    System.out.println("(2)View Data");
	    System.out.println("(3)Quit");
	    if( powermode ) {
		System.out.println("**(4)Connect to Server");
	    }
	    int s = IO.promptMenuOption("Select: ", option);	
	    
	    switch(s){
	    case -1:
		if( powermode) setPowermode(false);
		else System.out.println("You Are Not In Powermode");
		break;
	    case 0:
		if( !powermode ) setPowermode(true);
		else System.out.println("Already in Powermode");
		break;
	    case 1:
	 	readBinFileMenu();
		break;
	    case 2:
		viewDataMenu();
		break;
	    case 3:
		logout();
                b = false;
                break;
	    case 4:
		connectServerMenu();
                break;
	    default:
		break;
	    }
	}//end while
    }//end mainmenu();
    
    private void connectServerMenu(){
	boolean b = true;
	while(b){
	    clr();
	    System.out.println(" ** POWER MODE ONLY FEATURES ** ");
	    System.out.println("Please Choose From 1 and 3");
	    System.out.println("(1)Use Default Info from Configure File");
	    System.out.println("(2)Enter Server Information Manually");
	    System.out.println("(3)Back To Main Menu");
	    //###############################################################
	    //CHANGE THE 2ND PARAMETER TO THE NUMBER OF OPTIONS
	    //###############################################################
	    int s = IO.promptMenuOption("Select: ", 3);	
	    switch(s){
	    case -1:
                if( powermode) setPowermode(false);
                else System.out.println("You Are Not In Powermode");
                break;
            case 0:
                if( !powermode ) setPowermode(true);
                else System.out.println("Already in Powermode");
                break;
	    case 1:
		System.out.println("Server Host: " + server +
				   "\nUsername: "+ username + 
				   "\nDatabse: " + database + "\n");
		conn = inout.connectDBServer(server,database, username, password, conn);
		if(conn == null) b = true;
		else b = false;
		break;
	    case 2:
		clr();
		server = IO.promptString("Server: ");
		database = IO.promptString("Databse: ");
		username = IO.promptString("Username: ");
		password = IO.promptString("Password: ");
		conn = inout.connectDBServer(server, database, username, password, conn);
		if(conn == null) b = true;
		else b = false;
		break;
		//###############################################################	
		//CHANGE THE FOLLOWING CASE VALUE WHEN MENU IS EDITED
		//###############################################################
	    case 3:
		b = false;
		break;
	    default:
		break;
	    }
	}//end while
    }
    
    
    private void readBinFileMenu(){
	String filename = null;
	String destination = null;
	String username = null;
	String password = null;
	String server = null;
	boolean hasConnection = true;
	boolean b = true;
	hasConnection = checkConnection();
	
	//prompt for file name to read and store into database			 
	while(b){
	    clr();
	    //###############################################################
            //CHANGE THE FOLLOWING CASE VALUE WHEN MENU IS EDITED
            //###############################################################
	    int option = 3;//regular number of option
	    if( powermode ){
		option = 3;//power number of optiosn
		System.out.println(" ** POWERMODE ** ");
	    }
	    System.out.println("Please Choose The Type of Binary File");
	    System.out.println("(1)TreeFile");
	    //System.out.println("(2)OtherFile");
	    System.out.println("(2)Show Existing Tables");
	    System.out.println("(3)Back to Main Menu");
	    
	    int s = IO.promptMenuOption("Select: ", option);    

	    switch(s){
	    case -1:
                if( powermode) setPowermode(false);
                else System.out.println("You Are Not In Powermode");
                break;
            case 0:
                if( !powermode ) setPowermode(true);
                else System.out.println("Already in Powermode");
                break;
	    case 1:
		filename = IO.promptString("Please enter the name of the binary file: ");
                destination = IO.promptString("Please name the table to store the data: ");
                //readBinFile returns false if did not read sucessfully
		if (!inout.readBinFile(filename, destination, conn, s))
                    System.out.println("Read binary file failed.\n Pleaes check the file name and file type.");
		break;
	    case 2:
		viewData vd = new viewData();
		vd.showTables(conn);
		break;
		//###############################################################
                //CHANGE THE FOLLOWING CASE VALUE WHEN MENU IS EDITED
                //###############################################################
	    case 3:
		b = false;
		break;
	    default:
		break;
	    }//end switch
	}//end while
    }
    
    
    private void viewDataMenu(){
	boolean hasConnection = checkConnection();
	boolean b = true;
	while(b){
	    clr();
	    //###############################################################
            //CHANGE THE FOLLOWING CASE VALUE WHEN MENU IS EDITED
            //###############################################################
	    int option = 4;
	    if( powermode ) {
		option = 6;
		System.out.println(" ** POWERMODE ** ");
	    }

	    System.out.println("Please choose from 1 to " + option);
	    System.out.println("(1)Chains of Data with Given kza");
	    System.out.println("(2)Minimum Tree with Given kza");
	    System.out.println("(3)Show Existing Tables");
	    System.out.println("(4)Back to Main Menu");
	    if( powermode ) {
		System.out.println("**(5)Tree Data as Table");
		System.out.println("**(6)Tree Data as Tree");
	    }
	    
	    int s = IO.promptMenuOption("Select: ", option);
	    
	    String tableName = null;
	    viewData vd = new viewData();
	    
	    //only proceed if not to go back to main menu
	    switch(s){
	    case -1:
                if( powermode) setPowermode(false);
                else System.out.println("You Are Not In Powermode");
                break;
            case 0:
                if( !powermode ) setPowermode(true);
                else System.out.println("Already in Powermode");
                break;
	    case 1:
		tableName = IO.promptString("Please enter the name of the table:");
		int kza = IO.promptInt("Please Enter the KZA: ");
		vd.givenKZA(tableName, conn, kza);
		break;
	    case 2:
		tableName = IO.promptString("Please enter the name of the table:");
		kza = IO.promptInt("Please Enter the KZA: ");
		vd.givenKZA_minTree(tableName, conn, kza);
		break;
	    case 3:
		vd.showTables(conn);
		break;
	    case 4:
		b = false;
		break;
	    case 5:
		tableName = IO.promptString("Please enter the name of the table: ");
                int numLine = IO.promptInt("Please enter the number of lines to be displayed a time: ");
		vd.tableOfTree(tableName, conn, numLine);
                break;
            case 6:
		tableName = IO.promptString("Please enter the name of the table:");
                vd.viewAsTree(tableName, conn);
                break;
	    default:
		b = true;
		break;
	    }//end switch    
	}//end while
	
    }
    
    private void logout(){
	try{
	    conn.close();
	}catch(SQLException sqlx){
	    System.out.println("Error Occure when loggin out at ui.logout() --> " + sqlx);
	}
    }
    
    
    
    private boolean checkConnection(){
    	//Checking for connection
	if(conn == null){
	    System.out.println(" ** Should not appear unless in POWERMODE ** ");
	    System.out.println("There is no connection to database server.\n" +
			       "Read in binary file requires a connection to a database server.\n\n");
	    int j = IO.promptMenuOption("Would you like to Connecto to a databse server now?\n" +
				   "(1)Yes\n" +
				   "(2)No\n   --> ",2);
	    
	    if ( j == 1){
		connectServerMenu();
		return true;
	    }else
		return false;
	}//end if
	return true;
    }//end checkConnection
    
    
    private void clr(){
    	for( int k= 0; k<5; k++){
	    System.out.println("\n");
    	}
    }
    
    public void setPowermode(boolean p){
	powermode = p;
    }

    
}

