## Commit规范

### 1. **提交信息的基本结构**

```
<类型>(<模块>): <简要描述>

[可选的更详细描述]

[可选的参考信息，例如关闭的 Issue 编号或相关 PR]
```

------

### 2. **提交类型**

常见的提交类型可以根据更改的性质来分类：

- **feat**: 新功能（feature）
- **fix**: 修复 bug
- **docs**: 文档更新（例如 README.md）
- **style**: 代码格式（不影响功能，例如代码缩进、分号等）
- **refactor**: 代码重构（既不新增功能也不修复 bug）
- **perf**: 性能优化
- **sync**: 同步代码
- **test**: 添加或修改测试用例
- **chore**: 构建过程或辅助工具的变更（不涉及源码和测试文件）
- **revert**: 回滚提交
- **build**: 构建相关的更改
- **ci**: 持续集成配置相关
- **deps**: 依赖更新

------

### 3. **提交信息的例子**

```text
feat(user): add login functionality
```

```text
fix(auth): resolve token expiration issue

The token expiration logic was incorrectly calculating timezones.
This update adjusts the logic to handle UTC offsets properly.

Fixes #123
```

```text
revert: feat(user): add login functionality

This reverts commit abc123456789.
```
