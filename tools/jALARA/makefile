###
# define the Java compiler and flags
###
JC = javac 
FLAGS = 

###
# Here are the rules.
###
jALARA.class: jALARA.java ui.class  
	$(JC) $(FLAGS) jALARA.java

ui.class: ui.java InOut.class IO.class viewData.class
	$(JC) $(FLAGS) ui.java

IO.class: IO.java
	$(JC) $(FLAGS) IO.java

InOut.class: TreeTableDB.class TreeFile.class InOut.java
	$(JC) $(FLAGS) InOut.java

TreeTableDB.class: TreeTableDB.java
	$(JC) $(FLAGS) TreeTableDB.java

TreeFile.class: TreeFile.java
	$(JC) $(FLAGS) TreeFile.java

viewData.class: viewData.java treeDS.class TreeTableDB.class 
	$(JC) $(FLAGS) viewData.java

treeDS.class: treeDS.java
	$(JC) $(FLAGS) treeDS.java

###
# run tests
###
test: 	jALARA.class
	java jALARA -cp=org





