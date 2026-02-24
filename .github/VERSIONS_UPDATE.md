# GitHub Actions - Versões Corrigidas

## Ações Atualizadas

As seguintes ações foram atualizadas para versões não-deprecated:

### Versões Usadas no Projeto

| Ação | Versão Antiga | Versão Nova | Motivo |
|------|---------------|------------|--------|
| `actions/checkout` | v3 | v4 | v3 está deprecated desde 2024 |
| `actions/upload-artifact` | v3 | v4 | v3 foi deprecado em Abril 2024 |
| `actions/download-artifact` | v3 | v4 | Consistência com v4 |
| `softprops/action-gh-release` | v1 | v2 | Melhor suporte e features |
| `jurplel/install-qt-action` | v3 | v3 | Ainda ativa - sem mudanças necessárias |
| `ilammy/msvc-dev-cmd` | v1 | v1 | Ainda ativa - sem mudanças necessárias |

## Problemas Resolvidos

### ❌ Erro Original
```
Error: This request has been automatically failed because it uses a deprecated version 
of `actions/upload-artifact: v3`. Learn more: 
https://github.blog/changelog/2024-04-16-deprecation-notice-v3-of-the-artifact-actions/
```

### ✅ Solução Aplicada
- Atualizou `actions/checkout@v3` → `actions/checkout@v4`
- Atualizou `actions/upload-artifact@v3` → `actions/upload-artifact@v4`
- Atualizou `softprops/action-gh-release@v1` → `softprops/action-gh-release@v2`

## Arquivos Atualizados

1. `.github/workflows/build.yml`
   - ✅ checkout@v4 em build-debug
   - ✅ checkout@v4 em build-release
   - ✅ upload-artifact@v4

2. `.github/workflows/build-release.yml`
   - ✅ checkout@v4
   - ✅ softprops/action-gh-release@v2

## Como Validar

Para verificar se está usando versões corretas:

```bash
grep -r "uses: actions/" .github/workflows/
grep -r "uses: softprops/" .github/workflows/
```

Saída esperada:
```
actions/checkout@v4
actions/upload-artifact@v4
softprops/action-gh-release@v2
```

## Referências Oficiais

- [GitHub Changelog - Artifact Upload/Download v4](https://github.blog/changelog/2024-02-13-github-actions-artifact-actions-now-support-uploading-artifacts/)
- [Actions/Checkout v4](https://github.com/actions/checkout)
- [softprops/action-gh-release v2](https://github.com/softprops/action-gh-release)

## Próximo Passo

Criar uma release com tag para testar os workflows atualizados:

```bash
git add .github/
git commit -m "Update GitHub Actions to use non-deprecated versions"
git push origin main

# Depois criar release
git tag v1.0.0
git push origin v1.0.0
```

---

**Atualizado**: 24 de fevereiro de 2026
