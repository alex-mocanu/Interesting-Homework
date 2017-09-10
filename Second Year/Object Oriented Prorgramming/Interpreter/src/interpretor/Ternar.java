package interpretor;

/**
 * Ternary node
 * 
 * @author alexm
 */
public class Ternar extends Nod {
	Ternar() {
		this(null);
	}

	Ternar(String val) {
		setName('N');
		this.setVal(val);
	}
}