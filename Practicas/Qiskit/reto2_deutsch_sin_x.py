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


def ejecutar_deutsch_sin_x(tipo: str):
    qc = QuantumCircuit(2, 1)
    # 1. Se omite la X sobre la ancilla: q1 permanece en |0> antes del Hadamard
    # qc.x(1)
    # 2. Hadamard en ambos cúbits: q0 -> |+>, q1 -> |+>  (no se prepara |->)
    qc.h(0)
    qc.h(1)
    # 3. Consulta única al oráculo U_f
    qc.compose(construir_oraculo(tipo), inplace=True)
    # 4. Interferencia Hadamard en q0 y medición
    qc.h(0)
    qc.measure(0, 0)
    counts = AerSimulator().run(qc, shots=1000).result().get_counts()
    return qc, counts


print("Deutsch SIN la compuerta X en la ancilla q1.")
print("ESPERADO: los cuatro oráculos miden q0 = '0', también los balanceados.")
print()

shots = 1000
for caso in ["constante_0", "constante_1", "balanceada_identidad", "balanceada_inversa"]:
    circ, res = ejecutar_deutsch_sin_x(caso)
    bit = max(res, key=res.get)
    conclusion = "CONSTANTE" if bit == "0" else "BALANCEADA"
    print(f"CIRCUITO ({caso})")
    print(circ.draw(output="text"))
    print(f"Medición q0: '{bit}' -> el algoritmo diría {conclusion}")
    for estado in ("0", "1"):
        n = res.get(estado, 0)
        print(f"  |{estado}⟩: {n} ocurrencias ({n / shots * 100:.2f}%)")
    print()

print("Justificación: sin la X, el Hadamard deja la ancilla en |+⟩ y no en |−⟩.")
print("|+⟩ no adquiere la fase (−1)^f(x), así que no hay phase kickback. q0")
print("sigue en |+⟩ para cualquier oráculo y el Hadamard final lo manda a |0⟩.")
print("Constantes y balanceadas quedan indistinguibles.")
