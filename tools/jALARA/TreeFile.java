/********** NEW ***********
/ reading bin file method now shows the percent of data to be read
/**************************

/*
 * @(#)TreeFile.java 1.0 01/10/19
 *
 * You can modify the template of this file in the
 * directory ..\JCreator\Templates\Template_1\Project_Name.java
 *
 * You can also create your own project template by making a new
 * folder in the directory ..\JCreator\Template\. Use the other
 * templates as examples.
 *
 */
 
import java.io.*;

class TreeFile
{
	
	public String fileName;
	private DataInputStream treeStream;
	
	TreeFile(String openFileName){
		fileName = openFileName;
		
		try{
			treeStream = new DataInputStream(new BufferedInputStream(new FileInputStream(fileName)));
			System.out.println("The file " + fileName + " has been successfully opened.\n");
		}catch(IOException iox){
			System.out.println("Error opening " + fileName);
		}
	}
	
	public void readFile(TreeTableDB tdb){
		int nodeNum = 0;
		int kza = 0; 
		int parNum = 0;
		double relProd = 0.0;
		int available = 0;
		int count = 5000;

		try{
			available = treeStream.available();
			int totalAvailable = available;
			while(available != 0){
				parNum = treeStream.readInt();
				nodeNum = treeStream.readInt();
				kza = treeStream.readInt();
				relProd = (double)treeStream.readFloat();
      				TreeData tdata = new TreeData(nodeNum, kza, parNum, relProd);
				tdb.insertNode(tdata);
				available = treeStream.available();
				if( count % 5000 == 0){
				    System.out.println( Math.floor((((double)available/(double)totalAvailable)*100)) + "% left to be read.");
				}
				count ++;
			
			}
		}catch(IOException iox){
			System.out.println("Unable to read from file and write to data members.\n");
		}
	}
	
	public void close(){
		try{
			treeStream.close();
			System.out.println("The file " + fileName + " has been successfully closed.\n");
		}catch(IOException iox){
			System.out.println("Error closing " + fileName);
		}
	}

/*	
	public static void main(String args[])
	{
		TreeFile tfile = new TreeFile("Trial");
		tfile.readFile("TrialTree");
		tfile.close();
	}
	*/
}
