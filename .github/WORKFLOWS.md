# GitHub Workflows - IDS_DEStruct

Este documento explica como usar os workflows automatizados para compilar e criar releases.

## Workflows Disponíveis

### 1. `build-release.yml` - Build + Release Automático

**Disparador**: Tags com formato `v*` (ex: `v1.0.0`, `v2.1.3-beta`)

**O que faz**:
- Compila em Release (Windows MSVC 2019)
- Packota binários + dependências (Qt5, Qwt)
- Cria arquivo 7z e ZIP
- Cria release no GitHub automaticamente

**Como usar**:
```bash
# 1. Fazer commit com as alterações
git add .
git commit -m "Descrição das alterações"

# 2. Criar tag
git tag v1.0.0

# 3. Fazer push da tag
git push origin v1.0.0
```

O workflow será automaticamente disparado e você verá:
- Build logs em GitHub Actions
- Artifacts criados (7z e zip)
- Release publicada em "Releases" do repositório

### 2. `build.yml` - Build Contínuo

**Disparador**: 
- Commits em `main` ou `develop`
- Pull requests
- Manual (`workflow_dispatch`)

**O que faz**:
- Compila em Debug
- Compila em Release
- Faz upload de artifacts (sem criar release)
- Útil para verificar se código está compilável

**Como usar**:
Automático em commits/PRs. Para disparo manual:
```
GitHub UI → Actions → Build Application → Run workflow
```

---

## Estrutura de Release

Quando você cria uma tag, o workflow empacota:

```
IDS_DEStruct-release/
├── bin/
│   ├── IDS_DEStruct.exe          (Executável)
│   ├── Qt5Core.dll               (Runtime Qt)
│   ├── Qt5Gui.dll
│   ├── Qt5Widgets.dll
│   ├── Qt5OpenGL.dll
│   ├── Qt5Svg.dll
│   └── Qt5PrintSupport.dll
├── lib/
│   ├── qwt.dll                   (Biblioteca Qwt)
│   ├── qwtd.dll
│   ├── Qt5Core.lib               (Import libs)
│   ├── Qt5Gui.lib
│   └── ...
├── plugins/
│   └── platforms/
│       └── qwindows.dll          (Platform plugin QT)
└── doc/
    ├── BUILD.md
    └── LOGICA_EVOLUTIVA.md
```

---

## Prerequisitos Locais (GitHub Actions)

O workflow instala automaticamente via `workflow_dispatch`:
- Qt 5.15.2 (MSVC 2019 64-bit)
- MSVC 2019
- 7-Zip (pré-instalado em `windows-latest`)

**Nenhuma configuração local necessária no GitHub.**

---

## Troubleshooting

### Release não foi criada
1. Verifique se a tag segue o padrão `v*` (ex: `v1.0.0`)
2. Vá em GitHub Actions e veja os logs de erro
3. Comum: Qt não instalou - retire e re-execute workflow

### Binários faltando nas releases
Verifique se os caminhos em `.github/workflows/build-release.yml` Match com sua estrutura local:
```yaml
copy qwt\lib\qwt.dll release\lib\
```

### Build falhou
Verifique os logs detalhados em:
```
GitHub.com/seu-usuario/IDS_DEStruct → Actions → Job logs
```

---

## Versioning

Recomenda-se usar [Semantic Versioning](https://semver.org/):
- `v1.0.0` - Primeira release (Major.Minor.Patch)
- `v1.0.1` - Patch/bugfix
- `v1.1.0` - Nova feature (minor)
- `v2.0.0` - Breaking change (major)

---

## Exemplos de Uso

### Release v1.0.0
```bash
git tag v1.0.0 -m "Initial release - ELS Extended with corrected equation"
git push origin v1.0.0
```

### Release com nota detalhada (opcional)
```bash
git tag v1.1.0 -m "Version 1.1.0: Improved convergence detection"
git push origin v1.1.0
```

Release será criada automaticamente com:
- Artifacts 7z e ZIP
- Release notes com data, commit, branch
- Links para baixar

---

## Como Customizar

### Adicionar mais dependências
Edit `.github/workflows/build-release.yml` seção "Copy libraries":
```yaml
- name: Copy additional libraries
  run: |
    copy extra\lib\*.dll release\lib\
  shell: cmd
```

### Mudar formato de package
Modifique as linhas de compressão:
```yaml
# Adicionar .exe zipado
7z a IDS_DEStruct-release-portable.exe release\
```

### Adicionar validation step
Antes de criar release, execute testes:
```yaml
- name: Run tests
  run: |
    cd build
    .\release\IDS_DEStruct.exe --test
```

---

## Ambiente de CI/CD

O GitHub Actions fornece:
- **Runner**: Windows Server 2022 (`windows-latest`)
- **Disk**: ~30GB disponível
- **RAM**: ~7GB
- **Tempo**: Até 6h por job (normal ~10-15min para este projeto)

---

## Status Badge (Opcional)

Adicione ao README.md:
```markdown
[![Build Application](https://github.com/seu-usuario/IDS_DEStruct/actions/workflows/build.yml/badge.svg)](https://github.com/seu-usuario/IDS_DEStruct/actions/workflows/build.yml)
```

---

## Próximos Passos

1. Commitar os workflows:
```bash
git add .github/workflows/
git commit -m "Add GitHub Actions workflows for CI/CD"
git push origin main
```

2. Criar primeira release:
```bash
git tag v1.0.0
git push origin v1.0.0
```

3. Verificar em:
```
GitHub.com/seu-usuario/IDS_DEStruct/releases
```

---

**Criado**: 24 de fevereiro de 2026  
**Última atualização**: 24 de fevereiro de 2026
