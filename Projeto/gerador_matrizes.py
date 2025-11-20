import random as ramdom

def gerar_matriz_esparsa(dimensao, esparsidade):
    '''Gera uma matriz esparsa representada como um dicionário.
    As chaves do dicionário são tuplas (linha, coluna)
    Os valores são os elementos não nulos da matriz.'''
    
    matriz = {}
    num_elementos = round(dimensao * dimensao * esparsidade)

    for _ in range(num_elementos):
        posicao_valida = False

        while posicao_valida == False:
            linha = ramdom.randint(0, dimensao - 1)
            coluna = ramdom.randint(0, dimensao - 1)

            if (linha, coluna) not in matriz:
                posicao_valida = True
                valor = ramdom.randint(1, 100)
                matriz[(linha, coluna)] = valor
    return matriz


def main():
    esparsidades = []
    for i in range(2,6):
        if i < 4:
            esparsidades = [0.01, 0.05, 0.1, 0.2]
        else:
            esparsidades = [
            (1 / 10**(i+2)) / 100,
            (1 / 10**(i+1)) / 100,
            (1 / 10**i) / 100
        ]

        for esparsidade in esparsidades:
            dimensao = 10 ** i
            matriz = gerar_matriz_esparsa(dimensao, esparsidade)
            print(f"Dimensao: {dimensao} x {dimensao} esparsidade de esparsidade: {esparsidade:.6f} - Número de elementos não nulos: {len(matriz)}")


if __name__ == "__main__":
    main()