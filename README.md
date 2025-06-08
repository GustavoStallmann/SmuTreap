
<img src="./logo.png" width="100%">

# 🌌 SmuTreap – Árvore Espacial de Prioridade

Projeto desenvolvido para a disciplina de **Estrutura de Dados II**, com temática inspirada no universo de **Star Wars**. A SmuTreap é uma árvore de prioridade espacial com **promoção dinâmica**, utilizada para gerenciar e consultar formas geométricas em um plano bidimensional.


## 📜 Descrição

O programa lê formas geométricas (círculos, retângulos, linhas e textos) a partir de um arquivo `.geo` e as armazena em uma estrutura SmuTreap. Em seguida, processa comandos de um arquivo `.qry`, que podem realizar:

- Consultas espaciais
- Modificações geométricas
- Ações temáticas como disparo de ogivas (`disp`), explosões (`blow`), espionagem (`spy`) e limpeza (`cln`)

Os resultados são gerados em:

- Arquivos de texto (`.txt`)
- Visualizações gráficas (`.svg`)
- Estrutura da árvore em formato `.dot` (Graphviz)


## ⚙️ Tecnologias Utilizadas

- **C (C99)** — Linguagem principal de desenvolvimento  
- **Makefile** — Automação da compilação  
- **SVG** — Visualização gráfica das formas e operações  
- **Graphviz (DOT)** — Visualização da estrutura da SmuTreap  


## 🚀 Como Executar

### Pré-requisitos

- GCC (compilador C)
- `make` instalado

### Passos

1. Clone o repositório:
   ```bash
   git clone <URL_DO_REPOSITORIO>
   cd <PASTA_DO_PROJETO>/src
   ```

2. Compile o projeto:
   ```bash
   make ted
   ```

3. Execute com os parâmetros desejados:
   ```bash
   ./ted -e ../testes -f entrada.geo -q consultas.qry -o ../saida
   ```

### Parâmetros

| Parâmetro | Descrição                                                                 |
|-----------|---------------------------------------------------------------------------|
| `-e`      | Diretório base de entrada                                                 |
| `-f`      | Arquivo `.geo` com a descrição das formas                                 |
| `-q`      | Arquivo `.qry` com as consultas (opcional)                                |
| `-o`      | Diretório de saída para os arquivos gerados (`.svg`, `.txt`, `.dot`)      |



## 🛰️ Funcionalidades

- **🌌 Estrutura SmuTreap**  
  Treap espacial com bounding boxes para otimização de buscas e promoção dinâmica de nós

- **🗺️ Consultas Espaciais**  
  Seleção eficiente de formas dentro de uma região retangular

- **💥 Operações Temáticas**  
  Comandos especiais para simular combates espaciais:  
  `disp`, `blow`, `spy`, `cln`, entre outros

- **🖼️ Visualização Gráfica**  
  Geração de arquivos `.svg` representando o campo de batalha antes e depois das ações

- **🌲 Visualização da Árvore**  
  Exportação da estrutura final da SmuTreap em `.dot` para análise com Graphviz

---

## 📁 Estrutura de Diretórios Recomendada

```
projeto/
│
├── src/              # Código-fonte
├── testes/           # Arquivos .geo e .qry para teste
├── saida/            # Saída gerada (.txt, .svg, .dot)
└── README.md
```