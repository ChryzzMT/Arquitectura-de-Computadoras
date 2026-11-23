from qiskit import QuantumCircuit
from qiskit_aer import AerSimulator


def construir_oraculo(tipo: str) -> QuantumCircuit:
    oraculo = QuantumCircuit(2, name=f"U_f ({tipo})")
    if tipo == "constante_0":
        pass  # f(x) = 0
    elif tipo == "constante_1":
        oraculo.x(1)  # f(x) = 1
    elif tipo == "balanceada_identidad":
        oraculo.cx(0, 1)  # f(x) = x
    elif tipo == "balanceada_inversa":
        oraculo.x(0); oraculo.cx(0, 1); oraculo.x(0)  # f(x) = NOT x
    return oraculo


def ejecutar_deutsch(tipo: str):
    qc = QuantumCircuit(2, 1)
    # 1. Preparar cúbit ancilla en |1>
    qc.x(1)
    # 2. Superposición dual: q0 -> |+>, q1 -> |-> (habilita Phase Kickback)
    qc.h(0)
    qc.h(1)
    # 3. Consulta ÚNICA al oráculo U_f
    qc.compose(construir_oraculo(tipo), inplace=True)
    # 4. Interferencia Hadamard en q0 y medición
    qc.h(0)
    qc.measure(0, 0)
    counts = AerSimulator().run(qc, shots=1000).result().get_counts()
    return qc, counts


esperado = {
    "constante_0": ("0", "CONSTANTE"),
    "constante_1": ("0", "CONSTANTE"),
    "balanceada_identidad": ("1", "BALANCEADA"),
    "balanceada_inversa": ("1", "BALANCEADA"),
}

# Probar los 4 casos posibles de funciones booleanas:
shots = 1000
for caso in ["constante_0", "constante_1", "balanceada_identidad", "balanceada_inversa"]:
    circ, res = ejecutar_deutsch(caso)
    bit = max(res, key=res.get)
    bit_esp, conclusion_esp = esperado[caso]
    conclusion = "CONSTANTE" if bit == "0" else "BALANCEADA"
    print(f"CIRCUITO ({caso}), esperado q0 = '{bit_esp}' -> {conclusion_esp}")
    print(circ.draw(output="text"))
    print(f"Medición q0: '{bit}' -> Función {conclusion}")
    for estado in ("0", "1"):
        n = res.get(estado, 0)
        print(f"  |{estado}⟩: {n} ocurrencias ({n / shots * 100:.2f}%)")
    print()

print("Justificación: la X y el Hadamard dejan la ancilla en |−⟩. El oráculo")
print("multiplica la rama |x⟩ por (−1)^f(x): es el phase kickback. Si f es")
print("constante esa fase es global y el segundo Hadamard devuelve 0. Si f es")
print("balanceada la fase es relativa y devuelve 1. Hace falta una sola consulta.")
