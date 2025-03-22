import globals from 'globals';
import pluginJs from '@eslint/js';
import pluginReact from 'eslint-plugin-react';
import pluginUnusedImports from 'eslint-plugin-unused-imports';
import pluginReactHooks from 'eslint-plugin-react-hooks';
import { defineConfig } from 'eslint-define-config';

export default defineConfig([
  { languageOptions: {
      globals: globals.browser,
      parserOptions: {
        ecmaVersion: 2021,
        sourceType: 'module',
      },
    },
  },

  pluginJs.configs.recommended,
  pluginReact.configs.flat.recommended,

  { files: ['src/**/*.jsx'],
    plugins: {
      'unused-imports': pluginUnusedImports,
      'react-hooks': pluginReactHooks,
    },
    "settings": {
      "react": {
        "version": "detect"
      }
    }, 
    rules: {
      'prefer-const': 'error',
      'no-const-assign': 'error',
      'semi': ['error', 'always'],
      'no-unused-vars': ['error', { vars: 'all', args: 'after-used', ignoreRestSiblings: false }],
      'no-var': 'error',
      'unused-imports/no-unused-imports': 'error',
      'react/react-in-jsx-scope': 'off',
      'react-hooks/exhaustive-deps': 'warn',
      'react-hooks/rules-of-hooks': 'error',
      'react/forbid-prop-types': 'off',
      'react/no-unused-prop-types': 'warn',
      'react/prop-types': 'off',
    },
  },
]);

