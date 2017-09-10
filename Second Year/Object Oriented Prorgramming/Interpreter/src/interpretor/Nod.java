package interpretor;

import java.util.*;

/**
 * Implements the general structure of a node in the expression tree.
 * 
 * @author alexm
 */
public class Nod {
	// Level on which the node is situated in the tree
	private int level;
	// Node type (Expression, Term, Factor, Ternary)
	private char name;
	// Value in the node (Operator, Variable, Constant)
	private String val;
	private Nod[] children;

	Nod() {
		children = new Nod[2];
	}

	/**
	 * Returns the level on which the not is situated in the tree.
	 * 
	 * @return the level on which the not is situated in the tree
	 */
	public int getLevel() {
		return level;
	}

	/**
	 * Sets the level of the node.
	 * 
	 * @param level
	 *            number representing the level in the tree
	 */
	public void setLevel(int level) {
		this.level = level;
	}

	/**
	 * Returns the content of the node.
	 * 
	 * @return the content of the node
	 */
	public String getVal() {
		return val;
	}

	/**
	 * Sets the content of the node.
	 * 
	 * @param val
	 *            operator, variable or constant
	 */
	public void setVal(String val) {
		this.val = val;
	}

	/**
	 * Returns the type of node (Expression, Term, Factor, Ternary).
	 * 
	 * @return the type of node (Expression, Term, Factor, Ternary)
	 */
	public char getName() {
		return name;
	}

	/**
	 * Sets the type of the node.
	 * 
	 * @param name
	 *            type of a node
	 */
	public void setName(char name) {
		this.name = name;
	}

	/**
	 * Returns the first child of the node.
	 * 
	 * @return the first child of the node
	 */
	public Nod getChild(int no) {
		return children[no];
	}

	/**
	 * Sets the first child of the node.
	 * 
	 * @param child
	 *            node
	 */
	public void setChild(Nod child, int no) {
		children[no] = child;
	}

	/**
	 * Takes a character as a parameter and determines whether or not it is an
	 * operator or a parenthesis.
	 * 
	 * @param x
	 *            character
	 * @return whether the character is an operator
	 */
	public boolean symbol(char x) {
		if (x == '=' || x == '>' || x == '?' || x == ':')
			return true;
		if (x == '+' || x == '-' || x == '*')
			return true;
		if (x == '(' || x == ')')
			return true;
		return false;
	}

	/**
	 * Takes an infix notation and transforms it into a Reverse Polish notation.
	 * 
	 * @param expression
	 *            infix notation
	 * @return Reverse Polish notation
	 */
	public ArrayList<String> postfix(String expression) {
		char[] exp = expression.toCharArray();
		// Stack used to construct the Reverse Polish notation
		Stack<String> s = new Stack<String>();
		// Queue in which we store the Reverse Polish Notation
		ArrayList<String> q = new ArrayList<String>();
		StringBuffer aux = new StringBuffer();

		for (int i = 0; i < exp.length;) {
			// If the character is not an operator or parenthesis
			if (!symbol(exp[i])) {
				// We decide whether or not the variable/constant has a sign
				if (i > 1 && (exp[i - 1] == '+' || exp[i - 1] == '-') && exp[i - 2] == '(')
					aux.append(exp[i - 1]);
				else if (i == 1 && (exp[i - 1] == '-' || exp[i - 1] == '+'))
					aux.append(exp[i - 1]);
				while (i < exp.length && !symbol(exp[i]))
					aux.append(exp[i++]);
				q.add(aux.toString());
				aux = new StringBuffer();
			}
			// If the character is an operator or parenthesis we have the
			// following cases
			else if (exp[i] == '(') {
				aux.append(exp[i++]);
				s.push(aux.toString());
				aux = new StringBuffer();
			} else if (exp[i] == ')') {
				String x = null;
				do {
					x = s.pop();
					if (!x.equals("("))
						q.add(x);
				} while (!x.equals("("));
				i++;
			} else if (exp[i] == '+' || exp[i] == '-') {
				if (i > 0 && exp[i - 1] != '(') {
					aux.append(exp[i++]);
					while (!s.isEmpty() && !s.peek().equals("("))
						q.add(s.pop());
					s.push(aux.toString());
					aux = new StringBuffer();
				} else
					i++;
			} else if (exp[i] == '*') {
				aux.append(exp[i++]);
				while (!s.isEmpty() && s.peek().equals("*"))
					q.add(s.pop());
				s.push(aux.toString());
				aux = new StringBuffer();
			} else if (exp[i] == '>' || exp[i] == '?') {
				aux.append(exp[i++]);
				s.push(aux.toString());
				aux = new StringBuffer();
			} else if (exp[i] == ':') {
				aux.append(exp[i++]);
				q.add(s.pop());
				s.push(aux.toString());
				aux = new StringBuffer();
			}
		}

		// We empty the stack from the remaining operators
		while (!s.empty())
			q.add(s.pop());

		return q;
	}
}