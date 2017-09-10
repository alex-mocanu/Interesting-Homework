package decorator;

import java.util.ArrayList;

import homeworkPP.Context;
import homeworkPP.Main;
import visitor.Visitor;

// If program
public class IfProg extends Prog {
	private String s;

	public String getS() {
		return s;
	}

	public IfProg() {
		s = "";
	}

	public IfProg(String s) {
		this.s = s;
	}

	@Override
	public void eval(Context c) {
		// Extract the condition and the branches
		String expr = s.substring(4, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Establish the type of condition
		Elem<Boolean> val;
		if (list[0].charAt(1) == '=')
			val = new EqualExpr(list[0]);
		else
			val = new CompExpr(list[0]);
		// Decide which branch to execute
		Boolean decision = val.eval(c);
		Prog p = type(list[decision ? 1 : 2]);
		p.eval(c);
	}

	// Accept function for Visitor design pattern
	public void accept(Visitor v, Context c, ArrayList<String> l) {
		// List for local variables inside the if branches
		ArrayList<String> l2 = new ArrayList<String>();
		// Begin visit of if program
		v.startVisit(this, c, l2);
		// Extract the condition and the branches
		String expr = s.substring(4, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Apply accept to the condition and the branches
		Elem.type2(list[0]).accept(v, c);
		Prog.type(list[1]).accept(v, c, l2);
		Prog.type(list[2]).accept(v, c, l2);
		// End visit of if program
		v.endVisit(this, c, l2);
	}
}
