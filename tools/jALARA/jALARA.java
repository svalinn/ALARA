import java.sql.*;

class jALARA {
	
	//defined as constants
	public int majorVersion = 0;
	public int minorVersion = 1;
	public int revision = 1;
	
	public static void main (String args[]){
		try{
			ui startjALARA = new ui();
			startjALARA.launch();
			
		//InOut test = new InOut();
		//	Connection conn;
		//	if(test.connectDBServer("ATOMICDB.ep.wisc.edu", "yuanfenk", "macboy",conn)){
		//		test.readBinFile("tree1_100.bin", "TrialTree2",1);

		//	test.closeDBConnection();}
			
		
		}catch(Exception ex){
			System.err.println(ex);
			System.err.println("Error Occured When Launching UI");
		}
		
		
	/*	
		try{
			System.out.println("Registering JDBC Drivers");
			Class.forName("oracle.jdbc.driver.OracleDriver");//register JDBC drivers
		}catch(Exception e){
			System.out.println("\n Class not found exception");
		}
	
		//Open a connection to the database, it is shown here 2 alternatives, there are
		//many kinds of formats. The three information given are : URL of database,
		//username, and password, there could be more information passed in.
		try{
			System.out.println("Connecting to Server");
			Connection conn = DriverManager.getConnection 
							("jdbc:oracle:oci8:slim/ftiatbase@ATOMICDB.ep.wisc.edu");
		//Connection conn1 = DriverManager.getConnection
             //    ("jdbc:oracle:oci8:@ATOMICDB.ep.wisc.edu","slim","ftiatbase");
    
    		System.out.println("Opening File");
	    	TreeTableDB tdb = new TreeTableDB("TrialTree2", conn);
	    	TreeFile binTree = new TreeFile("tree1_100.bin");
	    	binTree.readFile(tdb);
	    	binTree.close();
	    	tdb.done();
	    	conn.close();
	    }catch(SQLException sqlx){
    		System.out.println("Error in database connectivity");
    	}		
	*/
	
	}	
	
}