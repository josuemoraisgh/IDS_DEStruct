# Como Compilar o IDS_DEStruct

## Requisitos
- Visual Studio 2019 (Build Tools, Community, Professional ou Enterprise)
- Qt 5.15.2 MSVC 2019 64-bit
- Qwt (já incluído em `qwt/`)

## Método 1: VS Code (Recomendado)

### Usar Tarefas Integradas
1. Abra o projeto no VS Code
2. Pressione `Ctrl+Shift+P` → "Tasks: Run Build Task" → `Build (Release) - MSVC 2019`
3. O executável estará em `build/release/IDS_DEStruct.exe`

### Atalhos de Teclado
- `Ctrl+Shift+B` - Executar build padrão (Release)

### Tarefas Disponíveis
- **Build (Release) - MSVC 2019** - Compilação otimizada
- **Build (Debug) - MSVC 2019** - Compilação com símbolos de depuração
- **Clean Build** - Limpar pasta build
- **Clean + Build (Release)** - Limpar e recompilar

## Método 2: Script PowerShell

```powershell
# Build Release
.\build-msvc.ps1

# Build Debug
.\build-msvc.ps1 -Debug

# Clean + Build
.\build-msvc.ps1 -Clean
```

## Método 3: Developer Command Prompt

1. Abra "Developer Command Prompt for VS 2019"
2. Navegue até a pasta do projeto
3. Execute:

```cmd
mkdir build
cd build
qmake ..\IDS_DEStruct.pro "CONFIG+=release"
nmake
```

## Método 4: Terminal do VS Code (com ambiente MSVC)

```powershell
# Configurar ambiente MSVC
cmd /k "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

# Depois executar:
mkdir build
cd build
qmake ..\IDS_DEStruct.pro "CONFIG+=release"
nmake
```

## Solução de Problemas

### Erro: 'type_traits': No such file or directory
**Causa:** Terminal sem variáveis de ambiente do MSVC  
**Solução:** Use o Método 1 (tarefas do VS Code) ou Método 2 (script PowerShell)

### Erro: qmake não encontrado
**Causa:** Qt não está no PATH  
**Solução:** 
1. Adicione Qt ao PATH: `C:\Qt\5.15.2\msvc2019_64\bin`
2. Ou use o script `build-msvc.ps1` que encontra automaticamente

### Erro: MSVC não encontrado
**Causa:** Visual Studio 2019 não instalado  
**Solução:** 
1. Instale VS 2019 Build Tools
2. Ou ajuste o caminho em `.vscode\tasks.json` e `build-msvc.ps1`

## Estrutura de Build

```
IDS_DEStruct/
├── build/               # Diretório de build (ignorado pelo git)
│   ├── obj/            # Arquivos objeto
│   ├── moc/            # Arquivos MOC do Qt
│   ├── ui/             # Headers gerados dos .ui
│   └── release/        # Executável final
│       └── IDS_DEStruct.exe
├── qwt/                # Biblioteca Qwt pré-compilada
│   ├── lib/           # DLLs do Qwt
│   └── src/           # Headers do Qwt
└── IDS_DEStruct.pro   # Arquivo de projeto Qt
```

## Notas

- O executável precisa das DLLs (Qt + Qwt) para rodar
- Para distribuição, use `windeployqt` para copiar as DLLs necessárias
- O GitHub Actions já faz isso automaticamente

## Build Estrito

Para habilitar warnings mais severos e tratar warnings como erro:

```cmd
mkdir build
cd build
qmake ..\IDS_DEStruct.pro "CONFIG+=debug strict_build"
nmake
```

No perfil `strict_build`, o projeto aplica `/W4`, `/WX` e `/permissive-` no MSVC.
