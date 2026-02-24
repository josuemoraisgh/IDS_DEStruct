# Como Criar uma Release com os Workflows

## Opção 1: GitHub Web UI (Recomendado - Mais Fácil)

1. Vá para: `https://github.com/josuemoraisgh/IDS_DEStruct/actions`
2. Na esquerda, clique em **"Build and Create Release"**
3. Clique no botão **"Run workflow"** (canto superior direito)
4. Preencha os campos:
   - **Versão/Tag** (obrigatório): `v1.0.0`, `v2.0.0-RC1`, etc
   - **Marcar como pré-release?**: ✅ ou ❌ (padrão: ❌)
   - **Notas da release** (opcional): Escreva o changelog ou deixe em branco
5. Clique em **"Run workflow"**

**Resultado**: O GitHub acionará automaticamente o workflow que irá:
- ✅ Compilar em Release
- ✅ Empacotar com Qt libraries e Qwt
- ✅ Criar arquivos 7z e ZIP
- ✅ Publicar release automática no GitHub

## Opção 2: Linha de Comando (GitHub CLI)

Se você tiver [GitHub CLI](https://cli.github.com/) instalado:

```bash
gh workflow run build-release.yml \
  -f version=v1.0.0 \
  -f prerelease=false \
  -f notes="Primeira release com equação ELS corrigida"
```

## Opção 3: Linha de Comando (Tradicional com Tags)

Se preferir manter o controle de versão via tags Git:

```bash
# 1. Sincronizar com main
cd c:\SourceCode\IDS_DEStruct
git checkout main
git pull origin main

# 2. Criar a tag
git tag v1.0.0 -m "Descrição da release"

# 3. Fazer push
git push origin v1.0.0

# 4. Acompanhar em GitHub Actions
# https://github.com/josuemoraisgh/IDS_DEStruct/actions
```

**Nota**: O workflow anterior também funcionava com tags, mas o novo usa `workflow_dispatch` que é mais intuitivo via UI.

## Verificar Status do Build

1. Ir para `https://github.com/josuemoraisgh/IDS_DEStruct/actions`
2. Selecionar o workflow "Build and Create Release"
3. Acompanhar o progresso em tempo real
4. Se sucesso ✅: Archive aparecerá em "Releases"
5. Se erro ❌: Verificar logs e corrigir

## Versioning Guide

Use [Semantic Versioning](https://semver.org/):

```
v1.0.0     ← Initial release
v1.0.1     ← Bug fix
v1.1.0     ← New feature
v2.0.0     ← Breaking change
v1.0.0-rc1 ← Release candidate
```

## O Que Está Incluído na Release

Cada release contém:

- **IDS_DEStruct.exe** - Executável principal
- **Qt 5.15.2 libraries** - Runtime (Core, Gui, Widgets, etc)
- **Qwt libraries** - Biblioteca de gráficos
- **plugins/** - Platform plugin Windows
- **doc/** - LOGICA_EVOLUTIVA.md + BUILD.md

**Formatos**:
- `.7z` - Mais comprimido (~30% menor)
- `.zip` - Mais compatível (qualquer OS)

## Troubleshooting

### "Release não apareceu após tag"
- Aguarde 1-2 minutos (workflow está processando)
- Se ainda não aparecer, vá em Actions e veja os logs
- Tag deve começar com `v` para disparar

### "Build failed no GitHub"
- Verifique os logs detalhados em Actions
- Comum: Qt não instalou - adicione retry ao workflow

### Quero testar antes de criar release
Use o workflow `build.yml`:
```bash
git commit -m "test" && git push origin main
```
Vai compilar debug/release sem criar release

## Próximos Releases

Depois do v1.0.0, para fazer v1.0.1:

```bash
# Fazer as alterações
git add .
git commit -m "Fix: tratamento de erro nos recursos"

# Criar tag
git tag v1.0.1
git push origin v1.0.1
```

---

**Dica**: Coloque os changelogs em um arquivo e referencie:

```
git tag v1.0.1 -m "$(cat CHANGELOG.md)"
```

