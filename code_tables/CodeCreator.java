package code_tables;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.Set;
import java.util.Stack;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class CodeCreator {
	
	public static final char CODE_X_HEX = 'D';
	public static final int CODE_X_VAL = 13;
	public static final int CODE_AMOUNT = 14;
	
	public static final int[] INPUTSYMBOLLIMIT = {	12, 	10, 	8, 		6, 
			6, 		4, 		4, 		4,
			2,		2, 		2, 		2, 
			2, 		2, 		2, 		0};

	public static final int[] THRESHOLD = 		  {	303336, 225404, 166979, 128672, 
				95597, 	69670, 	50678, 	34898, 
				23331, 	14935, 	9282, 	5510, 
				3195, 	1928, 	1112, 	408};
	
	@SuppressWarnings("unchecked")
	static TreeTable<Codeword>[] tables = (TreeTable<Codeword>[]) new TreeTable<?>[16];
	@SuppressWarnings("unchecked")
	static TreeTable<TreeTable<Codeword>>[] reverseTables = (TreeTable<TreeTable<Codeword>>[]) new TreeTable<?>[16];
	@SuppressWarnings("unchecked")
	static TreeTable<TreeTable<Codeword>>[] reverseFlushTables = (TreeTable<TreeTable<Codeword>>[]) new TreeTable<?>[16];
	
	
	static {
		//create tables
		for (int i = 0; i < 16; i++) {
			tables[i] = createTable(RawCodeTables.lecs[i], RawCodeTables.flecs[i]);
			reverseTables[i] = createReverseTable(tables[i], false);
			reverseFlushTables[i] = createReverseTable(tables[i], true);
		}
		//generate IDs for each and every table
		Queue<TreeTable<Codeword>> tablequeue = new LinkedList<TreeTable<Codeword>>();
		for (int i = 0; i < 16; i++) {
			tablequeue.add(tables[i]);
		}
		int tableIndex = 0;
		while (!tablequeue.isEmpty()) {
			TreeTable<Codeword> tq = tablequeue.poll();
			//if (!tq.isTerminal()) {
				tq.id = tableIndex;
				tableIndex++;
			//}
			for (TreeTable<Codeword> child: tq)
				tablequeue.add(child);
		}
	}
	
	public static void checkAllTableIntegrity() {
		//check full tree table integrity
		for (int i = 0; i < 16; i++) {
			tables[i].checkFullTree(INPUTSYMBOLLIMIT[i], 1);
			reverseTables[i].checkFullTree(2, 0);
			reverseFlushTables[i].checkFullTree(2, 0);
		}
		System.out.println("All tables are full trees");
		
		//check that every node is reachable in reverse and only once
		for (int i = 0; i < 16; i++) {
			Set<TreeTable<Codeword>> dtset = new HashSet<>();
			Stack<TreeTable<Codeword>> dtstack = new Stack<>();
			//create set of unique tables
			dtstack.push(tables[i]);
			while(!dtstack.isEmpty()) {
				TreeTable<Codeword> ctt = dtstack.pop();
				if (dtset.contains(ctt))
					throw new IllegalStateException();
				dtset.add(ctt);
				for (TreeTable<Codeword> tt: ctt)
					dtstack.push(tt);
			}
			//check that all are reachable
			Stack<TreeTable<TreeTable<Codeword>>> reverseStack = new Stack<>();
			reverseStack.push(reverseTables[i]);
			reverseStack.push(reverseFlushTables[i]);
			while (!reverseStack.isEmpty()) {
				TreeTable<TreeTable<Codeword>> crt = reverseStack.pop();
				if (crt.isTerminal()) {
					//remove its object from the dtset
					if (!dtset.contains(crt.getValue()))
						throw new IllegalStateException();
					dtset.remove(crt.getValue());
				} else {
					for (TreeTable<TreeTable<Codeword>> child: crt)
						reverseStack.push(child);
				}
			}
			//check that all were visited
			if (!dtset.isEmpty())
				throw new IllegalStateException();
		}
		
		System.out.println("All reverse tables correctly point to all direct tables");
	}
	
	

	private static Stack<TreeTable<?>> generateCRepr(TreeTable<?>[] baseTables, String baseName) {
		//generate IDs for each and every table
		//we are creating new tables
		Stack<TreeTable<?>> tableStack = new Stack<TreeTable<?>>();
		Stack<TreeTable<?>> tableStackHelper = new Stack<TreeTable<?>>();
		int id = 0;
		for (int i = 15; i >= 0; i--) {
			TreeTable<?> tab = baseTables[i];
			tab.id = i;
			tableStack.add(tab);
		}
		id = 16;
		
		while (!tableStack.isEmpty()) {
			TreeTable<?> tq = tableStack.pop();
			tableStackHelper.push(tq);
			Stack<TreeTable<?>> tableStackHelper2 = new Stack<TreeTable<?>>();
			for (TreeTable<?> child: tq) {
				child.id = id++;
				tableStackHelper2.push(child);
			}
			while (!tableStackHelper2.isEmpty())
				tableStack.push(tableStackHelper2.pop());
		}

		if (tableStackHelper.peek().getValue() instanceof Codeword) {
			for (TreeTable<?> tt: tableStackHelper) {
				Codeword codeword = (Codeword) tt.getValue();
				String cw_output = "CodeWord codeword_" + tt.id + " = {.cw_value = " + codeword.getValue() + ", .cw_bits = " + codeword.getBits() + "};";
				System.out.println(cw_output);
			}
		}

		String temp = "TreeTable ";
		int it = 0;
		for (TreeTable<?> tt: tableStackHelper) {
			if(it++ == tableStackHelper.size() - 1){
				temp += baseName + "_" + tt.id + ";";
			} else {
				temp += baseName + "_" + tt.id + ", ";
			}
			if (it % 16 == 0) {
				System.out.println(temp);
				temp = "";
			}
		}
		System.out.println(temp);

		/*while (!tableStackHelper.isEmpty()) {
			tableStack.push(tableStackHelper.pop());
		}*/

		//TreeTable a = {.children = {&table0, &table1, NULL}, .parent = &table0, .size = 15, .parent_index = 4, .object = &cw0};
		for (TreeTable<?> tq: tableStackHelper) {
			String table_output = "TreeTable " + baseName + "_" + tq.id + " = {";
			if (!tq.isTerminal()) {
				String children_output = "TreeTable * " + baseName + "_" + tq.id + "_children [] = {";
				for (int i = 0; i < tq.getSize(); i++) {
					TreeTable<?> child = tq.getChild(i);
					if (child == null)
					children_output += "NULL";
					else {
						//tablequeue.add(child);
						children_output += "&" + baseName + "_" + child.id + "";
					}
					if (i != tq.getSize() - 1)
					children_output += ", ";
				}
				children_output += "};";
				System.out.println(children_output);
				table_output += ".children = " + baseName + "_" + tq.id + "_children, ";
			} else {
				table_output += ".children = NULL, ";
			}
			if (tq.isRoot())
				table_output += ".parent = NULL, ";
			else
				table_output += ".parent = &" + baseName + "_" + tq.getParent().id + ", ";

			if (tq.getValue() == null)
				table_output += ".parent_index = " + tq.getParentIndex() + ", .object = NULL};";
			else if (tq.getValue() instanceof Codeword)
				table_output += ".parent_index = " + tq.getParentIndex() + ", .object = &codeword_" + tq.id + "};";
			else //instance of TreeTable<Codeword>
				table_output += ".parent_index = " + tq.getParentIndex() + ", .object = &table_" + ((TreeTable<?>)tq.getValue()).id + "};";
			System.out.println(table_output);
		}
		return tableStackHelper;
	}

	public static String pad(String string, String pad) {
	  return (pad + string).substring(string.length());
	}
	
	
	public static long buildCode(long metacode, long entrylength, long entrybits, long nextaddr) {
		return ((metacode) << 36) | (entrylength << 31) | (entrybits << 10) | nextaddr;
	}
	
	public static void main(String[] args) {
		//print and check integrity
		/*for (int i = 0; i < 16; i++) {
			System.out.println("Table for " + i + ": " + tables[i]);
			System.out.println("Rtabl for " + i + ": " + reverseTables[i]);
			System.out.println("RFtab for " + i + ": " + reverseFlushTables[i]);
		}
		System.out.println("All tables built");*/
		
		//checkAllTableIntegrity();
		//generateVHDLRepr();
		System.out.println("//Tables");
		generateCRepr(tables, "table");
		System.out.println("//Reverse flush tables");
		generateCRepr(reverseFlushTables, "reverse_flush_table");
		System.out.println("//Reverse tables");
		generateCRepr(reverseTables, "reverse_table");
	}

	


	public static TreeTable<Codeword>[] getCodeTables() {
		return tables;
	}
	
	public static TreeTable<TreeTable<Codeword>>[] getReverseFlushTables() {
		return reverseFlushTables;
	}
	
	public static TreeTable<TreeTable<Codeword>>[] getReverseTables() {
		return reverseTables;
	}

	
	private static List<Pair<String, String>> parseInput(String rawInputTable) {
		//separate all input prefix and output prefix
		String basePattern = "([^,\\s]+|[^,]+,\\s[^\\s]+)\\s([^\\s]+)[\\s]?";
		List<Pair<String, String>> allMatches = new ArrayList<Pair<String, String>>();
		Matcher mbase = Pattern.compile(basePattern).matcher(rawInputTable);
		while (mbase.find()) {
			String input = mbase.group(1);
			input = input.replace('X', CODE_X_HEX); //replace X with D's to parse later better
			if (input.contains("null")) //replace nulls with empty sequences
				input = "";
			String output = mbase.group(2);
			allMatches.add(new Pair<String, String>(input, output));
		}
		
		//postprocess step by step	
		//first replace 'r' by all its values
		List<Pair<String, String>> allMatchesTemp = new ArrayList<Pair<String, String>>();
		for (Pair<String, String> entry: allMatches) {
			
			Pattern patternR = Pattern.compile("0\\^\\{r}([^,]+)?,\\s(\\d+)≤r≤(\\d+)");
			Matcher m = patternR.matcher(entry.first());
			if (m.matches()) {
				//need to match the second part as well
				Pattern patternRsec = Pattern.compile("<(\\d+).h\\(([0-9A-F]+)\\+(\\d+)?r\\)>");
				Matcher m2 = patternRsec.matcher(entry.second()); 
				if (!m2.matches()) {
					throw new IllegalStateException("Error matching");
				}
				//need to re-generate all of the new possibilities
				String firstPattern = m.group(1) == null ? "" : m.group(1);
				int lowBound = Integer.parseInt(m.group(2));
				int highBound = Integer.parseInt(m.group(3));
				int outBits = Integer.parseInt(m2.group(1));
				int base = Integer.parseInt(m2.group(2), 16);
				int multiplier = m2.group(3) == null ? 1 : Integer.parseInt(m2.group(3));
				for (int i = lowBound; i <= highBound; i++) {
					String inputCode = createZeroString(i) + firstPattern;
					int outputCodeNum = base + multiplier*i;
					outputCodeNum = BitTwiddling.reverseBits(outputCodeNum, outBits);
					String outputCode = "" + outBits + "'h" + Integer.toHexString(outputCodeNum);
					allMatchesTemp.add(new Pair<String, String>(inputCode, outputCode));
				}
			} else {
				allMatchesTemp.add(entry);
			}
		}
		allMatches = allMatchesTemp;
		
		//now replace 0^{\d+} occurences by the raw value
		for (Pair<String, String> entry: allMatches) {
			Pattern fixedRepPattern = Pattern.compile("0\\^\\{(\\d+)\\}(.*)");
			Matcher mfrep = fixedRepPattern.matcher(entry.first());
			if (mfrep.matches()) {
				int numzeros = Integer.parseInt(mfrep.group(1));
				String input = "";
				for (int i = 0; i < numzeros; i++) 
					input += "0";
				
				input += mfrep.group(2); 
				entry.setFirst(input);
			}
		}
		
		return allMatches;
	}
	
	private static TreeTable<Codeword> createTable(String TreeTable, String terminalTable) {

		List<Pair<String, String>> allMatches = parseInput(TreeTable);
		 
		TreeTable<Codeword> ct = new TreeTable<Codeword>(null, CODE_AMOUNT, 0);
		for (Pair<String, String> entry: allMatches) {
			String sequence = entry.first();
			String code = entry.second();
			//System.out.println(sequence + "->" + code);
			TreeTable<Codeword> currentTable = ct;
			for (int i = 0; i < sequence.length(); i++) {
				int node = Integer.parseInt(""+sequence.charAt(i), 16);
				if (i == sequence.length() - 1) {
					//this is the last, add code to code table
					Codeword cw = new Codeword(code);
					currentTable.addTerminalNode(node, cw);
				} else {
					//go through the table
					currentTable = currentTable.getNextOrAddDefault(node);
				}
			}
		}
		
		allMatches = parseInput(terminalTable);
		for (Pair<String, String> entry: allMatches) {
			String sequence = entry.first();
			String code = entry.second();
			//System.out.println(sequence + "->" + code);
			TreeTable<Codeword> currentTable = ct;
			for (int i = 0; i < sequence.length(); i++) {
				int node = Integer.parseInt(""+sequence.charAt(i), 16);
				currentTable = currentTable.getNextOrAddDefault(node);
			}
			currentTable.setValue(new Codeword(code));
		}

		return ct;
	}
	
	private static String createZeroString(int length) {
		String ret = "";
		for (int i = 0; i < length; i++) {
			ret += "0";
		}
		return ret;
	}
	
	
	private static TreeTable<TreeTable<Codeword>> createReverseTable(TreeTable<Codeword> tabs, boolean flush) {
		TreeTable<TreeTable<Codeword>> rt = new TreeTable<TreeTable<Codeword>>(null, 2, 0);
				
		Stack<TreeTable<Codeword>> tts = new Stack<TreeTable<Codeword>>();
		tts.push(tabs);
		
		while (!tts.isEmpty()) {
			TreeTable<Codeword> currentTable = tts.pop();
			//add terminal code only if table is terminal
			if (!flush && !currentTable.isTree() || flush & currentTable.isTree()) {
				Codeword val = currentTable.getValue();
				//System.out.println("Adding cw: " + val);
				addReverseTableEntry(rt, val, currentTable);
			}
			for (TreeTable<Codeword> child: currentTable) {
				tts.push(child);
			}
			
		}
		return rt;
	}
	
	private static void addReverseTableEntry(TreeTable<TreeTable<Codeword>> rt, Codeword val, TreeTable<Codeword> reference) {
		Iterator<Bit> itbit = val.reverseIterator();
		Bit bit = itbit.next();

		int cnt = 0;
		int sfx = 0;
		//go deep until it is terminal
		while(itbit.hasNext()) {
			sfx |= (bit.toInteger() << cnt);
			cnt++;
			try {
				rt = rt.getNextOrAddDefault(bit.toInteger());
			} catch (Exception e) {
				System.err.println("<" + e.getMessage() + "> "+ val + " has a suffix already processed: " + cnt + "'h" + Integer.toHexString(sfx));
				return;
			}
			bit = itbit.next();
		}
		//add the terminal node that references the input codeword table
		try {
			rt.addTerminalNode(bit.toInteger(), reference);
		} catch (Exception e) {
			System.err.println(reference.getValue() + " is a suffix of an already processed code");
			return;
		}
	}
	
}

/*
 * 
 * 	
	/*
	 * ([0-9A-Z]+)\ (\d+)['’]h([0-9A-Z]+)\s*
	 * lowent_0_code.add(new Pair<Codeword, Codeword>(new Codeword("$1"), new Codeword($2, "$3")));\n
	 * ([^,\s]+|[^,]+,\s[^\s]+)\s([^\s]+)[\s]?
	 *
 * 			String fixedRepPattern = "0\\^\\{(\\d+)\\}(.*)";
			Matcher mfrep = Pattern.compile(fixedRepPattern).matcher(input);
			if (mfrep.matches()) {
				int numzeros = Integer.parseInt(mfrep.group(1));
				input = "";
				for (int i = 0; i < numzeros; i++) 
					input += "0";
				
				input += mfrep.group(2); 
			}
			
	*/

