import java.util.Vector;

//This class Uses TreeData to create a tree data structure. 

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
    //
    //     note: variable level search vertially in the tree 
    //           variable i in the for loop search horizontally in the tree
    //
    //Pass: given -> a vector of nodes
    //      level -> the depth of the node
    //      tn -> the current node
    //Return: none
    //*************************************************************
    public void insertChain(Vector given){
	insertChain(given, 0, dummy);
    }

    private void insertChain(Vector given, int level,  TreeData td){
	//See $$ below
	boolean flag = false;
	
	//all the nodes in the given chain are already in the tree
	//insertChain is always called with 0 for the first round,
	//so don't worry that some given Vectors are not even being 
	//processed. this is only used to end the recursive call
	if( level >= given.size() ) return;
       
	//if the current node has no children then there is no
	//need to compare the given vector with the existing tree
	//because the whole vector should be added to the tree
	if( td.hasChildren()){
	    Vector tempChildren = td.getChildren();
	    TreeData tempTD = (TreeData)given.get(level);

	    for( int i = 0; i < tempChildren.size(); i++){
		if(  ((TreeData)tempChildren.get(i)).compareTreeData(tempTD) ){
		    // $$: if current given node == current tree node at this level
		    //     the rest of the nodes in the given chain will be
		    //     taken care in the next level of the recursive call
		    //     so add the rest of the chain should not be executed
		    //     when breaks out of the loop
		    //     same logic, the loop should not go on if finds identical nodes at this level
		    //     therefore breaks after the recursive call
		    flag= true;
		    insertChain(given, level+1, (TreeData)tempChildren.get(i));
		    break;
		}
	    }//end for
	}//end if

	//add the rest of the chain
	if(!flag){
	    for( int l = level ; l < given.size(); l++){
		TreeData theTemp = (TreeData)given.get(l);
		td.getChildren().add( new TreeData( theTemp.getNodeNum(), theTemp.getKza(), theTemp.getParNum(), theTemp.getRelProd(), l));
		Vector temp2 = td.getChildren();
		td = (TreeData)temp2.get(temp2.size()-1);
	    }//end for
	}
    }//end insertChain();

    //*************************************************************
    //Method: print DFS()
    //Use: ouput the nodes with depth first search
    //Pass: none
    //Return: none
    //************************************************************
    public void printDFS(int kza){
	printDFS(dummy, kza);
    }
		 
    private void printDFS(TreeData td, int kza){	
	//dummy node has no data
	if( !td.isDummy()){
	    int l = td.getLevel();
	    for( int ll = l; ll > 0; ll--)
		System.out.print("  |");
	    System.out.print("  |-> kza:" + td.getKza() + " RelProd:" + td.getRelProd());
	    //Set Marker to the node w/ given kza
	    if( td.getKza() == kza )
		System.out.println(" ** ");
	    else System.out.println("");
	}

	if( td.hasChildren() ){
	    int numChild = td.numChild();
	    Vector temp = td.getChildren();
	    for( int i = 0; i < numChild; i++)
       		printDFS( (TreeData)temp.get(i),kza );
	}

	return;

    }
	       
	

}






