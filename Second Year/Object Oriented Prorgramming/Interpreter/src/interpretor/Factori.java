package interpretor;

/**
 * Factor node
 * 
 * @author alexm
 */
public class Factori extends Nod {
	Factori() {
		this(null);
	}

	Factori(String val) {
		setName('F');
		this.setVal(val);
	}
}