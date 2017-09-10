package visitor;

import java.util.ArrayList;

import decorator.AddExpr;
import decorator.AttProg;
import decorator.CompExpr;
import decorator.EqualExpr;
import decorator.IfProg;
import decorator.MultExpr;
import decorator.NormalProg;
import decorator.ReturnProg;
import decorator.Value;
import decorator.WhileProg;
import homeworkPP.Context;
import homeworkPP.Main;

// Concrete visitor for checking the correctness of the program
public class CheckingVisitor implements Visitor {

	// If the a variable is not found in the context, the program is wrong
	@Override
	public void visit(Value x, Context c) {
		try {
			if (x.getS().charAt(0) < '0' || x.getS().charAt(0) > '9')
				c.valueOf(x.getS());
		} catch (RuntimeException e) {
			Main.correctProgram = false;
		}
	}

	// For visit and startVisit, we just decrease the number of parenthesis
	// For endVisit, we remove the local variables from the context
	@Override
	public void visit(AddExpr x, Context c) {
		Main.paranthesis--;
	}

	@Override
	public void visit(MultExpr x, Context c) {
		Main.paranthesis--;
	}

	@Override
	public void visit(EqualExpr x, Context c) {
		Main.paranthesis--;
	}

	@Override
	public void visit(CompExpr x, Context c) {
		Main.paranthesis--;
	}

	@Override
	public void visit(NormalProg x, Context c, ArrayList<String> l) {
		Main.paranthesis--;
	}

	@Override
	public void visit(AttProg x, Context c, ArrayList<String> l) {
		Main.paranthesis--;
	}

	@Override
	public void startVisit(IfProg x, Context c, ArrayList<String> l) {
		Main.paranthesis--;
	}

	@Override
	public void endVisit(IfProg x, Context c, ArrayList<String> l) {
		for (String key : l)
			c.remove(key);
	}

	@Override
	public void startVisit(WhileProg x, Context c, ArrayList<String> l) {
		Main.paranthesis--;
	}

	@Override
	public void endVisit(WhileProg x, Context c, ArrayList<String> l) {
		for (String key : l)
			c.remove(key);
	}

	@Override
	public void visit(ReturnProg x, Context c, ArrayList<String> l) {
		Main.paranthesis--;
	}
}
