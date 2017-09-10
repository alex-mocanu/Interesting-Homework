package interpretor;

import java.util.*;

/**
 * Expression node
 * 
 * @author alexm
 */
public class Expresii extends Nod {
	Expresii() {
		this(null);
	}

	Expresii(String val) {
		setName('E');
		this.setVal(val);
	}

	/**
	 * Takes a Reverse Polish notation and constructs the expression tree.
	 * 
	 * @param postfix
	 *            Reverse Polish notation
	 * @return root of the expression tree
	 */
	public Expresii createTree(ArrayList<String> postfix) {
		Stack<Expresii> s = new Stack<Expresii>();
		for (int i = 0; i < postfix.size(); i++) {
			if (postfix.get(i).length() == 1 && symbol(postfix.get(i).charAt(0))) {
				Expresii aux1 = s.pop();
				Expresii aux2 = s.pop();
				Expresii aux3 = new Expresii(postfix.get(i));
				aux3.setChild(aux2, 0);
				aux3.setChild(aux1, 1);
				s.push(aux3);
			} else {
				Expresii aux = new Expresii(postfix.get(i));
				s.push(aux);
			}
		}
		return s.pop();
	}

	/**
	 * Evaluates the result of an expression. It receives an expression tree and
	 * a hash of variables in order to evaluate the expression.
	 * 
	 * @param node
	 *            node whose expression tree we want to evaluate
	 * @param hash
	 *            hash of variables with their values
	 * @return the result of the expression
	 */
	public int evalTree(Nod node, HashMap<String, Integer> hash) {
		// If the node is not an operator we return its values
		if (node.getName() != 'E') {
			if (node.getVal().charAt(0) == '-' && hash.containsKey(node.getVal().substring(1)))
				return -hash.get(node.getVal().substring(1));
			else if(node.getVal().charAt(0) == '+' && hash.containsKey(node.getVal().substring(1)))
				return hash.get(node.getVal().substring(1));
			else if(hash.containsKey(node.getVal()))
				return hash.get(node.getVal());
			else
				return Integer.parseInt(node.getVal());
		}
		// If the node is an operator we continue the evaluation accordingly
		if (node.getVal().equals("+"))
			return evalTree(node.getChild(0), hash) + evalTree(node.getChild(1), hash);
		if (node.getVal().equals("-"))
			return evalTree(node.getChild(0), hash) - evalTree(node.getChild(1), hash);
		if (node.getVal().equals("*"))
			return evalTree(node.getChild(0), hash) * evalTree(node.getChild(1), hash);
		if (node.getVal().equals(">"))
			return (evalTree(node.getChild(0), hash) > evalTree(node.getChild(1), hash)) ? 1 : 0;
		if (node.getVal().equals("?"))
			return evalTree(node.getChild(0), hash) == 1 ? evalTree(node.getChild(1), hash) : Integer.MIN_VALUE;
		if (node.getVal().equals(":"))
			return evalTree(node.getChild(0), hash) == Integer.MIN_VALUE ? evalTree(node.getChild(1), hash)
					: evalTree(node.getChild(0), hash);
		return 0;
	}
}