/*
 * @(#)TreeTableDB.java 1.0 01/09/30
 *
 * You can modify the template of this file in the
 * directory ..\JCreator\Templates\Template_1\Project_Name.java
 *
 * You can also create your own project template by making a new
 * folder in the directory ..\JCreator\Template\. Use the other
 * templates as examples.
 *
 */
 
/******************* NEW ***** ********************
9/25/02
Because it's not alway the case that when creating an object 
of TreeTableDB a new table must to be made, therefore
a new consturctor is implemented just to take connection 
as a parameter.
A New method selectTable is alway impleamted
if no table is selected when the object of treetabledb is created
then can use selectTable to select table
and because the code is repeated, the original 
consturctor( that takes table name and conn as param) nose
uses selectTable to determine if there is already 
a table w/ the same name
***************************************************/

/******************* BUG FIXED ********************

7/12/02
1)for both getChildren and getKza
    while(rs.next()) are changed to do-while(rs.next())

9/23/02
treeTableDB's contructor now checks for exiting table
if the table exist the user have the option 
to name another table or use the existn one
if the table did not exist the table will be created
***************************************************/

/******************* CHANGE ***********************
7/10/02
change connecting command for mySQL from oracle
7/15/02
chagne the constructor of TreeTableDB
since mysQL has the command "create table if not exists
only one line of command is suffecient to do the 
work

8/12/02
TreeTableDB::getKza() 
changed to get data back order by kza 

9/03/02
fields in treeData changed from public to private
and older direct use of the fields now changed to
use the getXXX() method to get desirable value
**************************************************/

/******************* pendding *********************
7/10/02
command for mySQL to check or existing table (DONE)
**************************************************/

import java.io.*;
import java.sql.*;
import javax.swing.*;
import java.util.Vector;

class TreeData{
    private int nodeNum;
    private int kza;
    private int parNum;
    private double relProd;
    private int level;
    private Vector child = new Vector();
    private boolean isDummy = false;

    //Constructor for the dummy root
    //The dummy contains no data but a vecotr of roots to the trees
    public TreeData(){
	isDummy = true;
	level = 0;
    }

    //Constructor for actual nodes of the tree structure
    //level -> level (depth) of the node
    TreeData(int nodeNum, int kza, int parNum, double relProd, int level){
        this.nodeNum = nodeNum;
        this.kza = kza;
        this.parNum = parNum;
        this.relProd = relProd;
	this.level = level;
    }
    //Constorucotr for uses other than constructing a tree
    TreeData(int nodeNum, int kza, int parNum, double relProd){
	this.nodeNum = nodeNum;
	this.kza = kza;
	this.parNum = parNum;
	this.relProd = relProd;
    }
	
    //TreeData(){
    //	nodeNum = kza = parNum = 0;
    //	relProd = 0.0;	
    //}
	
	public int getNodeNum(){
		return nodeNum;	
	}
	
	public int getKza(){
		return kza;
	}
	
	public int getParNum(){
		return parNum;
	}
	
	public double getRelProd(){
		return relProd;	
	}
	
	public void setNodeNum (int nodeNum){
		this.nodeNum = nodeNum;	
	}
	
	public void setKza (int kza){
		this.kza = kza;
	}
	
	public void setParNum(int parNum){
		this.parNum = parNum;
	}
	
	public void setRelProd(double relProd){
		this.relProd = relProd;	
	}
	
	public String toString(){
		String treeData = "The values are: " + "nodeNum: " + nodeNum + ", kza: " + kza +
						  ", parNum: " + parNum + ", relProd: " + relProd;
		return treeData;
	}

    //Returns true if the node has children
    //        false otherwise
    public boolean hasChildren(){
        if (child == null) return false;
        else return true;
    }

    //Return the vector of Children the  node has
    public Vector getChildren(){
        return child;
    }

    //Return the number of Childrne the nodes has
    public int numChild(){
        return child.size();
    }

    //Return the level of the node
    public int getLevel(){
        return level;
    }

    //Pass: a TreeData
    //Return: true if the param is the same obejct as
    //        the TreeData the node contains
    //        false otherwise
    public boolean compareTreeData(TreeData t){
        if ( getKza() == t.getKza() ) return true;
        else return false;
    }

    public boolean isDummy(){
	return isDummy;
    }

}

class TreeTableDB{
    private Statement stmt;
    public String tableName;
    
    
    public  TreeTableDB(Connection conn){
	try{
	    stmt = conn.createStatement(ResultSet.TYPE_SCROLL_INSENSITIVE,ResultSet.CONCUR_UPDATABLE);
	}catch(SQLException s){
            System.err.println("Exception occured in the constructor: " + s);
        }
    }

    public  TreeTableDB(String tableName, Connection conn){
	
	this.tableName = tableName;
	
	try{	
	    // exitConstructor is a necessary modification of original code so 
	    // the user have the option to choose using existed table or not
	    boolean exitConstructor = false;
	    while( !exitConstructor ){
		stmt = conn.createStatement(ResultSet.TYPE_SCROLL_INSENSITIVE,
					    ResultSet.CONCUR_UPDATABLE);
		//stmt.execute("CREATE TABLE IF NOT EXISTS " + tableName + 
		//      " (nodeNum Integer(2), kza Integer(6), parNum Integer(2), relProd Double)");
		
		
		boolean exist = false;
		///the follwoing was ORACLE COMMAND that was first used
		//ResultSet rs = stmt.executeQuery("SELECT TNAME FROM TAB");	
		
		//ResultSet rs = stmt.executeQuery("Show tables");
		
		//check first if a table with the particular tableName
		//already exsisted in the database, if not create the table
		//while(rs.next()){
		//String exist_table = rs.getString(1);
		//  if(tableName.equalsIgnoreCase(exist_table))
		//exist = true;
		//}
		
		exist = selectTable(tableName);

		//if it hasn't existed yet and wants to create tbale
		if(!exist){
		    System.out.println("Create table " + tableName);
		    stmt.execute("CREATE TABLE " + tableName + 
				 " (nodeNum INTEGER(2), kza INTEGER(6), parNum INTEGER(2), relProd DOUBLE)");
		    exitConstructor = true;
		}//end if
			//table exist and want to createTable
		else {
		    IO io = new IO();
		    int opt = io.promptMenuOption("Table " + tableName + "is already in the database.\n" +
						  "Do you want to use another name?\n 1) YES 2) NO --> ",2);
		    if( opt == 1){
			tableName = io.promptString("Please name the table storing the data: ");
		    }
		    else exitConstructor = true;
		}
		
	    }//end while  
	    
	}catch(SQLException s){
	    System.err.println("Exception occured in the constructor: " + s);
	}
	
    }
    
    public boolean selectTable(String tableName){
	try{
	    ResultSet rs = stmt.executeQuery("Show tables");

	    //check first if a table with the particular tableName
	    //already exsisted in the database, if not create the table
	    while(rs.next()){
		String exist_table = rs.getString(1);
		if(tableName.equalsIgnoreCase(exist_table)){
		    this.tableName = tableName;
		    return true;
		}
	    }
	    return false;
	}catch(SQLException s){
	    System.err.println("Unexpected error occured when checking table int the data base. treeTableDB::hasTable"+s);
	    return false;
	}
    }

    public int insertNode(TreeData nodeData){
	try{
	    String command = "INSERT INTO " + tableName + " VALUES (" + nodeData.getNodeNum() + "," +
		nodeData.getKza() + "," + nodeData.getParNum() + "," + nodeData.getRelProd() + ")";
	    stmt.executeUpdate(command);
	}catch(SQLException s){
	    System.out.println(s);
	    System.out.println("The insertion failed. Method returns 0");
	    return 0;//false
	}
	return 1;//true;
    }
    
    public TreeData getNode(int nodeNum){
	TreeData td = new TreeData();
	try{
	    String command = "SELECT * FROM " + tableName + " WHERE nodeNum = " + nodeNum;
	    ResultSet rs = stmt.executeQuery(command);
	    if(rs.next()){
		td = new TreeData(rs.getInt("nodeNum"), rs.getInt("kza"), rs.getInt("parNum"), rs.getDouble("relProd"));
		//System.out.println(td);
	    }
	    else
		td = null;
	}catch(SQLException e){
	    System.out.println(e);
	}
	return td;	
    }
    
    TreeData[] getChildren(int parNum){
	TreeData[] tdArray;
	try{
	    String command = "SELECT * FROM " + tableName + " WHERE parNum = " + parNum;
	    //System.out.println(command);
	    ResultSet rs = stmt.executeQuery(command);
	    rs.last();
	    int num_rows = rs.getRow();
	    
	    if(num_rows == 0)
		return null;
	    
	    rs.first();
	    rs.previous();
	    
	    tdArray = new TreeData[num_rows];
	    for(int j=0; j<tdArray.length; j++)
		tdArray[j] = new TreeData();
	    
	    int i=0;
	    
	    do{
		TreeData td = new TreeData(rs.getInt("nodeNum"), rs.getInt("kza"), rs.getInt("parNum"), rs.getDouble("relProd"));	
		tdArray[i] = td;
		i++;
		//System.out.println("i=" + i);
	    }while(rs.next());
	    
	    return tdArray;
	    
	}catch(SQLException e){
	    System.out.println("TreeTableDB.getChildren()" + e);
	    return null;
	}
	
    }
    
    TreeData[] getKza(int kza){
	TreeData[] tdArray;
	try{
	    String command = "SELECT * FROM " + tableName + " WHERE kza = " + kza + " ORDER by relProd DESC";
	    //System.out.println(command);
	    ResultSet rs = stmt.executeQuery(command);
	    rs.last();
	    int num_rows = rs.getRow();
	    if(num_rows == 0)
		return null;
	    rs.first();
	    rs.previous();
	    tdArray = new TreeData[num_rows];
	    for(int j=0; j<tdArray.length; j++)
		tdArray[j] = new TreeData();
	    
	    int i=0;
	    
	    do{
		TreeData td = new TreeData(rs.getInt("nodeNum"), rs.getInt("kza"), rs.getInt("parNum"), rs.getDouble("relProd"));	
		//System.out.println(td);
		tdArray[i] = td;
		i++;	
	    }while(rs.next());
	    return tdArray;
	    
	}catch(SQLException e){
	    System.out.println("TreeTableDB.getKza: " +e);
	    return null;
	}
    }
    
    
    void done(){
	try{
	    stmt.close();
	}catch(SQLException e){
	    System.out.println("Can not close the statement object in method done");
	}
    }
    
    /*	
      public static void main(String[] args){			
      String tableName = "TrialTree";
      TreeTableDB treeDB = new TreeTableDB(tableName);
      TreeData dum = new TreeData(15, 5, 7, 5.5);
      //treeDB.insertNode(dum);
      //treeDB.getNode(10);
      //treeDB.getKza(57);
      treeDB.getChildren(8);
      treeDB.done();
      }
    */
}

class treeDS{

    //The dummy node (root) that will contains
    //all the roots of the actual trees
    private TreeData dummy;

    //Constructor to initiat the dummy node
    public  treeDS(){
        dummy = new TreeData();
    }

    //**************************************************************
    //Method: insertChain
    //Use: insert the pssed param vecotr of nodes into the tree
    //     the passed param is being compared with the current node
    //     (A)
    //     if they are identical then traverse to the chidren of current node
    //        if continue until there is no more nodes in the given vector
    //           if there is nodes left in the given vector when at leaf
    //              insert the rest of the given vector
    //     if not identical then traverse to other nodes of the same level
    //        if finds identical nodes goes back to (A)
    //        else add the rest of the nodes in the given vector
    //Pass: given -> a vector of nodes
    //      level -> the depth of the node
    //      tn -> the current node
    //Return: none
    //*************************************************************
    public void insertChain(Vector given){
        insertChain(given, 0, dummy);
    }
    private void insertChain(Vector given, int level, TreeData tn){
        //See $$ below
	boolean flag = false;

        //all the nodes in the given chain are already in the tree
        if( level >= given.size() ) return;

        //if the current node has no children then there is no
        // ** IMPORTANT ** this method compares the children
        //                 meaning once get into the method as the
        //                 current node, the current given node
        //                 and current tree nodes are already identical
        if( tn.hasChildren() ){
            Vector tempTN = tn.getChildren();
            TreeData tempTD = (TreeData)given.get(level);

            for( int i = 0; i < tempTN.size(); i++){
                if( ((TreeData)tempTN.get(i)).compareTreeData(tempTD) ){
                    // $$: if current given node == current tree node
                    //     the rest nodes in the given chain will be
                    //     taken care in the next level of the recursive call
                    //     so add the rest of the chain should not be executed
                    //     when breaks out of the loop
                    //     same logic, the loop should not go on if finds identical nodes
                    //     therefore breaks at after the recursive call
                    flag= true;
                    insertChain(given, level+1, (TreeData)tempTN.get(i));
                    break;
                }
            }//end for
        }//end if
	if(!flag){
            //add the rest of the chain
            for( int l = level ; l < given.size(); l++){
		TreeData td = (TreeData)given.get(l);
                tn.getChildren().add( new TreeData(td.getNodeNum(), td.getKza(), td.getParNum(), td.getRelProd(), l));
                Vector temp = tn.getChildren();
                tn = (TreeData)temp.get(temp.size()-1);
            }//end for
        }
    }//end insertChain();

    //*************************************************************
    //Method: print DFS()
    //Use: ouput the nodes with depth first search
    //Pass: none
    //Return: none
    //************************************************************
    public void printDFS(){
        printDFS(dummy);
    }

    private void printDFS(TreeData tn){
        boolean isDummy = tn.isDummy();

        //dummy node has no data
        if( !isDummy ){
            int l = tn.getLevel();
            //System.out.println("level = " + l);
            for( int ll = l; ll > 0; ll--)
                System.out.print("  |");
            System.out.println("  |-> kza:" + tn.getKza() + " RelProd:" + tn.getRelProd());
        }

        if( tn.hasChildren() ){
            int numChild = tn.numChild();
            Vector temp = tn.getChildren();
            for( int i = 0; i < numChild; i++)
                printDFS( (TreeData)temp.get(i) );
        }

        return;
    }

}





