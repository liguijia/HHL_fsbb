% 读取 XLSX 文件
filename = 'data.xlsx';
data = readtable(filename);

% 提取列名
colNames = data.Properties.VariableNames;

% 初始化 k 和 b 的存储以及组名
k_values = zeros(5, 1);
b_values = zeros(5, 1);
group_names = cell(5, 1); % 创建一个单元数组来存储组名

% 创建表格来存储结果，包括组名
fitResults = table('Size', [5 3], 'VariableTypes', {'cell', 'double', 'double'}, ...
    'VariableNames', {'GroupName', 'Slope_k', 'Intercept_b'});

% 初始化绘图
figure;

% 遍历每组数据
for i = 1:5
    % 提取 x 和 y 列
    xCol = colNames{i + 5};
    yCol = colNames{i};
    
    x = data.(xCol)(2:end);
    y = data.(yCol)(2:end);
    
    % 线性拟合
    p = polyfit(x, y, 1);
    k_values(i) = p(1);
    b_values(i) = p(2);
    
    % 存储到表格中，包括组名
    group_names{i} = sprintf('%s-%s', xCol, yCol); % 组合x列名和y列名为组名
    fitResults.GroupName{i} = group_names{i};
    fitResults.Slope_k(i) = k_values(i);
    fitResults.Intercept_b(i) = b_values(i);
    
    % 绘制原始数据点
    subplot(2, 3, i);
    scatter(x, y, 'filled');
    hold on;
    
    % 绘制拟合直线
    x_fit = linspace(min(x), max(x), 100);
    y_fit = polyval(p, x_fit);
    plot(x_fit, y_fit, 'r-', 'LineWidth', 2);
    
    % 添加标题和标签
    title(['拟合: ' yCol]);
    xlabel(xCol);
    ylabel(yCol);
    
    % 添加拟合参数
    text(max(x), max(y), sprintf('y = %.2fx + %.2f', k_values(i), b_values(i)), 'VerticalAlignment', 'Top', 'HorizontalAlignment', 'Right');
end

% 显示所有图表
hold off;

% 输出 k 和 b 的值及组名
disp('拟合得到的斜率(k)、截距(b)及其对应的组名如下：');
disp(fitResults);

% 如果需要将结果保存为文件，可以使用 writetable 函数
outputFilename = 'fit_results.xlsx';
writetable(fitResults, outputFilename);
disp(['拟合结果已保存至: ', outputFilename]);

