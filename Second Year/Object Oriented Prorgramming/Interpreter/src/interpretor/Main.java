package interpretor;

import java.io.*;
import java.util.*;

/**
 * Main class used to do all the operations
 * 
 * @author alexm
 */
public class Main {
	// Method used to decide if a character is a digit
	private static boolean number(char x) {
		if ('0' <= x && x <= '9')
			return true;
		return false;
	}

	// Method used to decide if parenthesis are needed for printing an
	// expression
	private static boolean paranthesisOperator(String x) {
		if (x.equals("+") || x.equals("-"))
			return true;
		if (x.equals(":") || x.equals(">"))
			return true;
		return false;
	}

	/**
	 * Depth first search function for changing the types of the leaves
	 * according to what they represent (Term, Factor, Ternary)
	 * 
	 * @param node
	 *            analyzed node
	 */
	public static void dfs(Nod node) {
		// Verify if the left child is a leaf
		if (node.getChild(0).getChild(0) == null && node.getChild(0).getChild(1) == null) {
			if (node.getVal().equals("=") || node.getVal().equals("+") || node.getVal().equals("-"))
				node.setChild(new Termeni(node.getChild(0).getVal()), 0);
			else if (node.getVal().equals("*"))
				node.setChild(new Factori(node.getChild(0).getVal()), 0);
			else if (node.getVal().equals("?") || node.getVal().equals(">") || node.getVal().equals(":"))
				node.setChild(new Ternar(node.getChild(0).getVal()), 0);
		} else
			dfs(node.getChild(0));

		// Verify if the right child is a leaf
		if (node.getChild(1).getChild(0) == null && node.getChild(1).getChild(1) == null) {
			if (node.getVal().equals("=") || node.getVal().equals("+") || node.getVal().equals("-"))
				node.setChild(new Termeni(node.getChild(1).getVal()), 1);
			else if (node.getVal().equals("*"))
				node.setChild(new Factori(node.getChild(1).getVal()), 1);
			else if (node.getVal().equals("?") || node.getVal().equals(">") || node.getVal().equals(":"))
				node.setChild(new Ternar(node.getChild(1).getVal()), 1);
		} else
			dfs(node.getChild(1));
	}

	/**
	 * Prints the tree structure. It uses a breadth first search in which for
	 * each level expressions are expanded.
	 * 
	 * @param expresie
	 *            root of the expression tree
	 * @param out
	 *            output file
	 */
	public static void bfs(Nod expression, PrintWriter out) {
		int level = 0;
		StringBuffer s = new StringBuffer();
		s.append("E");
		out.println(s);
		LinkedList<Nod> q = new LinkedList<Nod>();
		q.addLast(expression);
		while (!q.isEmpty()) {
			Nod x = q.getFirst();
			q.removeFirst();
			if (x.getLevel() == level + 1) {
				level++;
				out.println(s);
			}
			if (x.getChild(0) != null) {
				String print = x.getChild(0).getName() + x.getVal() + x.getChild(1).getName();
				// If the operator is +,-,> or :, we need parenthesis for the
				// expanded form of the expression
				if (paranthesisOperator(x.getVal()))
					s.replace(s.indexOf("E"), s.indexOf("E") + 1, "(" + print + ")");
				else
					s.replace(s.indexOf("E"), s.indexOf("E") + 1, print);
				x.getChild(0).setLevel(level + 1);
				x.getChild(1).setLevel(level + 1);
				q.addLast(x.getChild(0));
				q.addLast(x.getChild(1));
			}
		}
	}

	/**
	 * The main function used to generate the expression tree, find semantic
	 * errors and evaluate expression.
	 * 
	 * @param args
	 *            command line arguments
	 * @throws IOException
	 */
	public static void main(String[] args) throws IOException {
		BufferedReader in = new BufferedReader(new FileReader(args[0]));
		PrintWriter out1 = new PrintWriter(new File("arbore.out"));
		PrintWriter out2 = new PrintWriter(new File("semantica.out"));
		PrintWriter out3 = new PrintWriter(new File("eval_expresie.out"));
		// The expression tree
		Expresii root = new Expresii();
		String expression;
		// The number of the expression
		int lineNumber = 0;
		// Hash of the variables and their values
		HashMap<String, Integer> unknowns = new HashMap<String, Integer>();

		// We read each line of the input file
		while ((expression = in.readLine()) != null) {
			lineNumber++;
			// We determine the left-hand side and right-hand side of the
			// expression and construct the tree
			String expression1 = expression.split("=")[0];
			String expression2 = expression.split("=")[1];
			ArrayList<String> postfix1 = root.postfix(expression1);
			ArrayList<String> postfix2 = root.postfix(expression2);
			root.setChild(root.createTree(postfix1), 0);
			root.setChild(root.createTree(postfix2), 1);
			root.setVal("=");
			dfs(root);
			bfs(root, out1);
			out1.println();

			// We determine semantic errors and evaluate the expression
			if (root.getChild(0).getName() != 'T' || number(root.getChild(0).getVal().charAt(0))
					|| root.symbol(root.getChild(0).getVal().charAt(0))) {
				out2.println("membrul stang nu este o variabila la linia " + lineNumber + " coloana 1");
				out3.println("error");
				continue;
			}

			boolean moveOn = false;
			String exp = expression.split("=")[1];
			int offset = expression.split("=")[0].length() + 2;
			for (int i = 0; i < exp.length() && !moveOn; i++) {
				StringBuffer term = new StringBuffer();
				if (!root.symbol(exp.charAt(i))) {
					int pos = i + offset;
					while (i < exp.length() && !root.symbol(exp.charAt(i)) && !number(exp.charAt(i)))
						term.append(exp.charAt(i++));
					if (!term.toString().equals("") && !unknowns.containsKey(term.toString())) {
						out2.println(term + " nedeclarata la linia " + lineNumber + " coloana " + pos);
						out3.println("error");
						moveOn = true;
					}
				}
			}
			// Decide whether we can move on to evaluation of the expression
			if (moveOn)
				continue;
			out2.println("Ok!");

			int result = root.evalTree(root.getChild(1), unknowns);
			unknowns.put(root.getChild(0).getVal(), result);
			out3.println(root.getChild(0).getVal() + "=" + result);
		}

		in.close();
		out1.close();
		out2.close();
		out3.close();
	}
}
