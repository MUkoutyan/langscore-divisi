// テストデータ (ツクールのゲームプロジェクト) の置き場所を解決する。
// サイズと再配布の都合でリポジトリには含めないため、既定ではリポジトリと同階層の
// langscore-divisi-test-data を参照する。環境変数 LANGSCORE_TEST_DATA で上書きできる。
const fs = require('fs');
const path = require('path');

const TEST_DATA_DIR_NAME = 'langscore-divisi-test-data';

const root = process.env.LANGSCORE_TEST_DATA
    ? path.resolve(process.env.LANGSCORE_TEST_DATA)
    : path.resolve(__dirname, '..', '..', '..', TEST_DATA_DIR_NAME);

// プラグイン用のゲームプロジェクト (mv_test / mz_test / vxace_test) の親フォルダ
const pluginRoot = path.join(root, 'plugin');

function requirePluginProject(name) {
    const projectPath = path.join(pluginRoot, name);
    if (!fs.existsSync(projectPath)) {
        throw new Error(
            `テストデータが見つかりません: ${projectPath}\n` +
            `  リポジトリと同階層に ${TEST_DATA_DIR_NAME} を配置するか、\n` +
            `  環境変数 LANGSCORE_TEST_DATA で場所を指定してください。`);
    }
    return projectPath;
}

module.exports = { root, pluginRoot, requirePluginProject };
