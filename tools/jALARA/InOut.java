//########## NEW #############
//9/23/02
//now checks if the file to read in exist or not\
//############################

//########## CHANGE ##############
//8/17/02
//connectDBServer: put everything into one try claus
//                 it just doesn't make sense to have to
//                 if there are two then even if the first part
//                 does not pass the 2nd part still executes !?
//################################

import java.sql.*;


class InOut{

    //****************************************************************
    //Method: ConnectDBServer
    //Use: make connection with server of user's choice
    //Pass: server -> server name
    //		user -> user name of the server
    //		pass -> password to the server
    //		conn -> the connection that is going to be returned
    //Return: 	conn -> connection make
    //			conn = null -> connection fail
    //**************************************************************** 
    public Connection connectDBServer(String server, String database, String user, String pass, Connection conn) {
	//register JDBC drivers
	try{
	    //System.out.println("Registering JDBC Drivers");
	    //Class.forName("oracle.jdbc.driver.OracleDriver");// for oracle driver
	    Class.forName("org.gjt.mm.mysql.Driver");//for mysql driver
	
	    //Open a connection to the database, it is shown here 2 alternatives, there are
	    //many kinds of formats. The three information given are : URL of database,
	    //username, and password, there could be more information passed in.
       
	    System.out.println("Connecting to Server..");
	    //Connection conn = DriverManager.getConnection(); 
	    //conn = DriverManager.getConnection ("jdbc:oracle:oci8:"+user+"/"+pass+"@"+server);
	    //"jdbc:mysql://127.0.0.1/alara", "laushu", "chiayu"
	    conn = java.sql.DriverManager.getConnection("jdbc:mysql://" + server + "/" + database, user, pass);
	    
	    System.out.println("Connection sucessfully made...");
        }catch(SQLException sqlx){
	    System.err.println("Error in database connectivity.\n" +
			       "Please check the username, password, hostname, or server connection.\n" +
			       "All Connections Have Benn Terminated.\n");
	    conn = null;
	    return conn;
    	}catch(Exception ex){
	    System.err.println("Unexpected Error when Connecting to DB Server: "+ex);
    	}
    	
    	return conn;
    	
    }//end connectDBServer()
    
    
    
    public Connection closeDBConnection(Connection conn){
    	if (conn != null){
    		try{
    			conn.close();
    		}catch(SQLException sqlx){
    			System.err.println("Error in database connectivity");
    		}catch(Exception ex){
    			System.err.println("Unexpected Error when closing DB Server: " + ex);
    		}
    	}
    	return conn;
    }
    
    
    
    public boolean readBinFile(String from, String to,Connection conn, int type ){
    	try{
	    java.io.File tempFile = new java.io.File(from);
	    if( tempFile.exists() ){
		System.out.println("Opening File");
		TreeTableDB tdb = new TreeTableDB( to, conn);
		TreeFile binTree = new TreeFile(from);
		binTree.readFile(tdb);
		binTree.close();
		tdb.done();
		System.out.println("File has been sucessfully read and stored into the database.\n");
		//return true;
	    }else {
		System.out.println("File " + from + " does not exist.");
		return false;
	    }
	}catch(Exception ex){
	    System.err.println("Unexpected Error when reading Binary file to send to database" + ex);
	    return false;
	}
	return true;
    }//end readBinFile
    
       	
}//end class
