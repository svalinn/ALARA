/******************* NEW *************************
9/23/02
implemented showTables()
9/25/02
-now checks if a particular tables exist before proceeding
in both viewChainw/KZA and also viewMinTreew/KZA
-in both viewChainw/KZA and also viewMinTreew/KZA
if the particular kza doesn't exit, it won't crash anymore
/******************* BUG FIXED ********************
8/13/02
viewTree does not use the  quickSort wiritten 
TreeTableDB::getKza now gets the TreeDatas that's
already oredred by relPro by sql so quickSort
does not cause null pont or out of bundry crash
***************************************************/

/******************* CHANGE ***********************
9/20/02
add numLine param into viewAsTable()
so it will display disire number of lines a time
9/23/02
-taking out toString() 
replace old use of toString w/ regular 
System.out.println();
-utilize IO.java's promptStirng and promptInt
so it won't crash
**************************************************/

/******************* pendding *********************
**************************************************/


import java.sql.*;
import java.io.*;
import java.util.Vector;
import java.util.TreeSet;

class viewData{
	
    private IO io = new IO();
	
    //*******************************************************************
    //Use: output data of a table specified by user and ouput as a table
    //     not in anyparticular order but how it was inserted into the
    //     data base initially
    //Pass: tableName -> the table desire to get data from
    //      conn      -> the connection to the server
    //      numLine   -> number of line to be displayed a time
    //Return: None
    //******************************************************************
    public void tableOfTree(String tableName, Connection conn, int numLine){
	try{
	    Statement stmt = conn.createStatement();
	    ResultSet rs = stmt.executeQuery( "SELECT * FROM " + tableName );
	    System.out.println(tableName);
	    System.out.println("NodeNum        Kza      ParNumber     RealProduct");
	    int count = 1;
	  
	    while (rs.next()) {
		if(count % numLine == 0){
		    String c = io.promptString("Press \"Enter\" to continue and \"q\" to quit: ");
		    if( c.compareTo("q") == 0 ) break;
		    System.out.println(tableName + "  (continue)" );
		    System.out.println("NodeNum	 Kza	  ParNumber 	RealProduct");
		}
		int nodeNum = rs.getInt("nodeNum");
		int kza = rs.getInt("kza");
		int parNum = rs.getInt("parNum");
		double relPro = rs.getDouble("relProd");
		System.out.println(nodeNum + "       " + kza +"   " + parNum + "       " + relPro);
		count++;
	    }
	}catch( SQLException sqlx ){
	    System.err.println("Error Occred in viewData.tableOfTree()\n" + sqlx);	
	}
    }//end tableOfTree
    
    //*******************************************************************
    //Use: return data of a table specified by user and output as a Tree
    //     not sorted in any particular way but how the data was inserted
    //     into the database initially
    //Pass: tableName -> the table desire to get data from
    //      conn      -> the connection to the server
    //      rank      -> the depth of tree start from 0
    //      index     -> within the same rank, index of different children
    //                   start from 0
    //Return: None
    //******************************************************************
    public void viewAsTree(String tableName, Connection conn){
	viewAsTree(tableName, conn, 0, 1);
    }//end viewAsTree
    
    private void viewAsTree(String tableName, Connection conn, int rank, int index){
        //FOR DEBUGGING
        //System.out.println("rank: " + rank);
        //System.out.println("NodeNum: " + index);
	
	TreeTableDB tdb = new TreeTableDB(conn);	
	if( !tdb.selectTable(tableName)){
	    System.out.println("There is no table named " + tableName );
	    return;
	}
	TreeData[] tdbArray = tdb.getChildren(index);


	//FOR DEBUGGING
	//System.out.println(tdbArray.length);
	if (tdbArray != null){
	    
	    for ( int k = 0; k < tdbArray.length; k ++){
		for(int j = 0; j <= rank-1; j++){ System.out.print("  |"); }
		
		//Next 2 lines  could be used to check if the output is correctg
		//System.out.println("  |->ParNum:" + tdbArray[k].getParNum() + " NodeNum:" + tdbArray[k].getNodeNum());
		//System.out.println("  |->"+tdbArray[k]);
		
		//Next Line for Real OutPut
		System.out.println("  |->KZA:" + tdbArray[k].getKza() + " RelProd:" + tdbArray[k].getRelProd());
		
		//check if there are children, if so call recursively, if not finish the for loop
		TreeData[] tdbArrayK = tdb.getChildren(tdbArray[k].getNodeNum());
		
		if (tdbArrayK != null)
		    viewAsTree(tableName, conn, rank+1, tdbArray[k].getNodeNum());
		
	    }//end For
	}
	
    }//end viewAsTree
    
    //****************************************************************
    //Method: givenKZA
    //Use: show all the paths from the root to nodes with the given Kza.
    //     Outputs result in serise of chains of nodes in DESC order
    //     in terms of relProd(relative product).
    //Pass: tableName -> the name of the table interested in
    //      conn -> the sql connection 
    //      kza -> the particular kza intersted
    //Return: none
    //****************************************************************
    public void givenKZA(String tableName, Connection conn, int kza){
	//get the nodes w/ the given kza
	TreeTableDB tdb = new TreeTableDB(conn);
	if( !tdb.selectTable(tableName) ){
	    System.out.println("There is no table" +  tableName);
            return;
        }
	TreeData[] kzaArray = tdb.getKza(kza);
	if( kzaArray == null ){
	    System.out.println("There is not particular kza with table" + tableName);
	    return;
	}
	System.out.println( kzaArray.length );
	System.out.println("There are " + kzaArray.length + " nodes with given kza: ");
	
	for (int k = 0; k<kzaArray.length; k++){
	    //search for the chain with the kth node found above
	    //from node to root
	    Vector v = new Vector();
	    v.add(kzaArray[k]);
	    int nodeNum = kzaArray[k].getParNum();
	    while(nodeNum != 0){
		TreeData td = tdb.getNode(nodeNum);
		v.add(td);
		nodeNum = td.getParNum();
	    }//end while
	    
	    int i = 0; //for priting pretty
	    //output the result 
	    for(int j = v.size()-1; j>=0; j--){
		i++;
		TreeData tem = (TreeData)v.get(j);
		System.out.print("kza:"+tem.getKza()+" RelProd:" + tem.getRelProd());
		//set marker to the nodes w/ given kza
		if (tem.getKza() == kza) System.out.println(" ** ");
		else System.out.println("");
		if( j != 0) System.out.print( " ---> " );
		else System.out.println("\n");
	    }
	}//end for
    }//end givenKZA
    
    //******************************************************************
    //Method: givenKZA_minTree
    //Use: given a specific kza interested, output all the paths from 
    //     the root to nodes that have the specific kza specified.
    //     duplicated paths are eliminated.  Outputs result as a tree.
    //Pass: tableName -> name of the table interestd
    //      conn -> sql connection
    //      kza -> the kza interested in
    //Return: none
    //*****************************************************************
    public void givenKZA_minTree(String tableName, Connection conn, int kza){
	//get the nodes w/ the given kza
	TreeTableDB tdb = new TreeTableDB(conn);
	if( !tdb.selectTable(tableName) ){
            System.out.println("There is no table" +  tableName);
            return;
        }
	TreeData[] kzaArray = tdb.getKza(kza);
	if( kzaArray == null ){
            System.out.println("There is not particular kza with table" + tableName);
            return;
        }
	//System.out.println("There are " + kzaArray.length + " nodes with given kza: ");	
	//create a new tree to store data
	treeDS theMinTree = new treeDS();
 
	for (int k = 0; k<kzaArray.length; k++){
	    //search for the chain with the kth node found above
	    //from node to root, found nodes are added to the front
	    //so the first node of the resulting chain starts from the root
	    Vector v = new Vector();
	    v.add(kzaArray[k]);
	    int nodeNum = kzaArray[k].getParNum();
	    while(nodeNum != 0){
		TreeData td = tdb.getNode(nodeNum);
		v.add(0,td);
		nodeNum = td.getParNum();
	    }//end while
	    theMinTree.insertChain(v);
	}//end for
     
	theMinTree.printDFS(kza);
	
    }//end givenKZA
    
    //******************************************************************
    //Method: showTables
    //Use: Show the tables names in the database
    //Pass: conn -> sql connection
    //Return: none
    //*****************************************************************
    public void showTables(Connection conn){
	try{
	    Statement stmt = conn.createStatement(ResultSet.TYPE_SCROLL_INSENSITIVE,
						  ResultSet.CONCUR_UPDATABLE);
            ResultSet rs = stmt.executeQuery("show tables");
            //java.sql.ResultSet rs = ds.getResultSet ();
            if (rs != null){
                System.out.println("Tables in Database");
                System.out.println("------------------");
                while (rs.next ()){
                    System.out.println(rs.getString (1));
                }
            }
        }catch(Exception ex){
            System.err.println("Unexpected Error Shwoing Table Names:" + ex);
        }
    }
    


}//end class viewData



















