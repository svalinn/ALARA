import java.sql.*;

class DataSet{
	public String dataSetName;	//the name of the dataset
	public int major_version;	//the minimum version of jALARA
				       //required for this dataset
	public int minor_version;
	public int revision;
	public String dateStamp;	//some kind of datestamp indicating when
					//this dataset was added
								
	public String treeTableName;//the name of the tree data table
	
	static String[] dataSetFields = {"Name", "Version", "DateStamp", "TreeTable"};
	
	DataSet(){
		dataSetName = dateStamp = treeTableName = " ";
		major_version = minor_version = revision = 0;	
	}
	
	DataSet(String name, int version, String date, String tree){
		dataSetName = name; 
		major_version = version;
		dateStamp = date;
		treeTableName = tree;	
	}
	
	public void setDataSetName(String name){
		dataSetName = name;	
	}
	
	public void setVersionNum(int version){
		major_version = version;	
	}
	
	public void setDateStamp(String date){
		dateStamp = date;	
	}
	
	public void setTreeTable(String table){
		treeTableName = table;
	}
	
	public String getDataSetName(){
		return dataSetName;	
	}
	
	public int getVersionNum(){
		return major_version;	
	}
	
	public String getDateStamp(){
		return dateStamp;	
	}
	
	public String getTreeTableName(){
		return treeTableName;
	}
	
	public boolean checkVersion(int codeVersionNum){
		if(major_version == codeVersionNum)
			return true;
		else
			return false;	
	}
	
	public static String[] getDataSetFields(){
		return dataSetFields;	
	}	
}

class TableOfContentsDB{
	
	Statement stmt;
	String ToCName;
	String[] ToCFields; //This contains an array of title for the
			    //different fields/columns of the table.
	
	
	//This is the default constructor.
	//It checks to see if the table given by the first argument exists.
	//If not, it creates such a table
	//If the table exists, it is queried for the set of fields/columns
	//that it supports and there are stored in dataSetFields.
	//If it does not exist, the fields/columns in these tables
	//are defined by the getToCFields method of DataSet					
	TableOfContentsDB(String newToCName, Connection thisConn){
		ToCName = newToCName;
		ToCFields = DataSet.getDataSetFields();
		
		try{	
			stmt = thisConn.createStatement(ResultSet.TYPE_SCROLL_INSENSITIVE,
                                      		ResultSet.CONCUR_UPDATABLE);
			boolean exist = false;
			ResultSet rs = stmt.executeQuery("SELECT TNAME FROM TAB");	
			
			//check first if a table with the particular tableName
			//already exsisted in the database, if not create the table
			while(rs.next()){
				String exist_table = rs.getString(1);
				if(ToCName.equalsIgnoreCase(exist_table))
					exist = true;
			}
			
			//if it hasn't existed yet
			if(!exist){
				stmt.execute("CREATE TABLE " + ToCName + " (" + ToCFields[0] + " VARCHAR(10), " + ToCFields[1] + "NUMBER(3), " + ToCFields[2] + "VARCHAR(10), " + ToCFields[3] + " VARCHAR(10))");
				System.out.println("Create table " + ToCName);
			}
			
		}catch(SQLException s){
			System.out.println("Exception occured in the constructor");
		}		
	}
	
	//This method inserts a record/row into the current table of contents.
	public int insertDataSet(DataSet data){
		try{
			String command = "INSERT INTO " + ToCName + " VALUES (" + data.dataSetName + "," +
							 data.major_version + "," + data.dateStamp + "," + data.treeTableName + ")";
			stmt.executeUpdate(command);
		}catch(SQLException s){
			System.out.println(s);
			System.out.println("The insertion failed. Method returns 0");
			return 0;//false
		}
		return 1;//true;	
	}
	
	//This method queries the database and returns a dataSet object with
	//a name that matches the argument.
	public DataSet getDataSetName(String dataSetName){
		DataSet d = new DataSet();
		
		try{
			String command = "SELECT * FROM " + ToCName + " WHERE Name = " + dataSetName;
			ResultSet rs = stmt.executeQuery(command);
			if(rs.next()){
				d = new DataSet(rs.getString("Name"), rs.getInt("Version"), rs.getString("DateStamp"), rs.getString("TreeTable"));
			}
			else
				d = null;
		}catch(SQLException e){
			System.out.println(e);
		}
		
		return d;	
	}
	
	//This method queries the database and returns an array of dataSet
	//objects, each with a datestamp that matches the argument.
	public DataSet[] getDataSetDated(String dateStamp){
		DataSet[] dArray;
		
		try{
			String command = "SELECT * FROM " + ToCName + " WHERE DateStamp = " + dateStamp;
			System.out.println(command);
			ResultSet rs = stmt.executeQuery(command);
			rs.last();
			int num_rows = rs.getRow();
			if(num_rows == 0)
				return null;
			rs.first();
			rs.previous();
			dArray = new DataSet[num_rows];
			for(int j=0; j<dArray.length; j++)
				dArray[j] = new DataSet();

			int i=0;

			while(rs.next()){
				DataSet d = new DataSet(rs.getString("Name"), rs.getInt("Version"), rs.getString("DateStamp"), rs.getString("TreeTable"));
				dArray[i] = d;
				i++;	
			}
			return dArray;
				
		}catch(SQLException e){
			System.out.println(e);
			return null;
		}	
	}
	
	public void done(){
		try{
			stmt.close();
		}catch(SQLException e){
			System.out.println("Can not close the statement object in method done");
		}
	}
}
