package visitor;

import java.util.ArrayList;

import decorator.*;
import homeworkPP.Context;

// Interface for Visitor design pattern
public interface Visitor {
	void visit(Value x, Context c);

	void visit(AddExpr x, Context c);

	void visit(MultExpr x, Context c);

	void visit(EqualExpr x, Context c);

	void visit(CompExpr x, Context c);

	void visit(NormalProg x, Context c, ArrayList<String> l);

	void visit(AttProg x, Context c, ArrayList<String> l);

	void startVisit(IfProg x, Context c, ArrayList<String> l);

	void endVisit(IfProg x, Context c, ArrayList<String> l);

	void startVisit(WhileProg x, Context c, ArrayList<String> l);

	void endVisit(WhileProg x, Context c, ArrayList<String> l);

	void visit(ReturnProg x, Context c, ArrayList<String> l);
}
