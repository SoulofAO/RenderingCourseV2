// Подключение заголовков Win32 API, DirectX 11, компилятора шейдеров и стандартных библиотек
#include <windows.h>
#include <WinUser.h>
#include <wrl.h>
#include <iostream>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <chrono>


// Подключение статических библиотек для линковки Direct3D 11
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


// Оконная процедура — обрабатывает сообщения, приходящие окну от ОС
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
	case WM_KEYDOWN:
	{
		// При нажатии клавиши выводим её код в консоль
		std::cout << "Key: " << static_cast<unsigned int>(wparam) << std::endl;

		// Escape (код 27) — завершаем приложение
		if (static_cast<unsigned int>(wparam) == 27) PostQuitMessage(0);
		return 0;
	}
	default:
		{
			// Все остальные сообщения передаём стандартному обработчику Windows
			return DefWindowProc(hwnd, umessage, wparam, lparam);
		}
	}
}


int main()
{
	LPCWSTR applicationName = L"My3DApp";
	HINSTANCE hInstance = GetModuleHandle(nullptr);

// �== СОЗДАНИЕ ОКНА ==ค
#pragma region Window init

	// Заполняем структуру класса окна — определяет поведение и внешний вид окна
    WNDCLASSEX wc;
    
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Перерисовка при изменении размера, собственный контекст устройства
	wc.lpfnWndProc = WndProc;						// Указатель на нашу оконную процедуру
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));	// Чёрный фон окна
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Регистрируем класс окна в системе
	RegisterClassEx(&wc);


	auto screenWidth = 800;
	auto screenHeight = 800;

	// Корректируем размер окна с учётом рамки и заголовка,
	// чтобы клиентская область была ровно 800x800
	RECT windowRect = { 0, 0, static_cast<LONG>(screenWidth), static_cast<LONG>(screenHeight) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	auto dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME;

	// Центрируем окно на экране
	auto posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
	auto posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

	// Создаём само окно
	HWND hWnd = CreateWindowEx(WS_EX_APPWINDOW, applicationName, applicationName,
		dwStyle,
		posX, posY,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr, nullptr, hInstance, nullptr);

	// Показываем окно, выводим на передний план и даём ему фокус ввода
	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	ShowCursor(true);

#pragma endregion Window init


// �== ИНИЦИАЛИЗАЦИЯ DIRECT3D 11 ==ค

	// Запрашиваем уровень возможностей DirectX 11.1
	D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };

	// Описание цепочки подкачки (swap chain) — управляет буферами, в которые рендерится изображение
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 2;									// Двойная буферизация (front + back buffer)
	swapDesc.BufferDesc.Width = screenWidth;
	swapDesc.BufferDesc.Height = screenHeight;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 32-битный цвет: 8 бит на канал (RGBA)
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;				// Частота обновления 60 Гц
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// Буфер используется как цель рендеринга
	swapDesc.OutputWindow = hWnd;								// Привязка к нашему окну
	swapDesc.Windowed = true;									// Оконный режим (не полноэкранный)
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// Современный режим показа: предыдущий кадр отбрасывается
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// Разрешаем переключение режимов отображения
	swapDesc.SampleDesc.Count = 1;								// Без мультисэмплинга (MSAA выключен)
	swapDesc.SampleDesc.Quality = 0;


	// Создаём устройство (device), контекст (context) и цепочку подкачки (swap chain) одним вызовом.
	// device    — представляет GPU, используется для создания ресурсов (буферов, текстур, шейдеров)
	// context   — используется для отправки команд рендеринга на GPU
	// swapChain — управляет переключением буферов и показом кадра на экране
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;

	auto res = D3D11CreateDeviceAndSwapChain(
		nullptr,					// Адаптер по умолчанию (основная видеокарта)
		D3D_DRIVER_TYPE_HARDWARE,	// Аппаратная акселерация (GPU)
		nullptr,					// Не используем программный растеризатор
		D3D11_CREATE_DEVICE_DEBUG,	// Включаем Debug-слой для отладки D3D-ошибок
		featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapDesc,
		&swapChain,
		&device,
		nullptr,
		&context);

	if(FAILED(res))
	{
	}

	// Получаем текстуру заднего буфера (back buffer) из swap chain
	// и создаём на неё Render Target View — это «вид», через который D3D будет рисовать в эту текстуру
	ID3D11Texture2D* backTex;
	ID3D11RenderTargetView* rtv;
	res = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTex);
	res = device->CreateRenderTargetView(backTex, nullptr, &rtv);


// �== КОМПИЛЯЦИЯ ШЕЙДЕРОВ ==ค

	// Компилируем вершинный шейдер (Vertex Shader) из HLSL-файла.
	// Точка входа: "VSMain", профиль шейдера: "vs_5_0" (Shader Model 5.0).
	// Флаги: отладочная информация + пропуск оптимизации (для удобства отладки)
	ID3DBlob* vertexBC = nullptr;
	ID3DBlob* errorVertexCode = nullptr;
	res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexBC,
		&errorVertexCode);

	if (FAILED(res)) {
		if (errorVertexCode) {
			// Если компиляция не удалась — выводим ошибку компилятора в консоль
			char* compileErrors = (char*)(errorVertexCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		else
		{
			// Если ошибки нет, но HRESULT провален — файл шейдера не найден
			MessageBox(hWnd, L"MyVeryFirstShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return 0;
	}

	// Макросы, которые передаются в HLSL при компиляции пиксельного шейдера:
	//   TEST  = 1              — включает условную ветку #ifdef TEST в шейдере
	//   TCOLOR = float4(...)   — определяет альтернативный цвет для правой половины экрана
	D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	// Компилируем пиксельный шейдер (Pixel Shader) из того же HLSL-файла.
	// Точка входа: "PSMain", профиль: "ps_5_0". Передаём макросы.
	ID3DBlob* pixelBC;
	ID3DBlob* errorPixelCode;
	res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl", Shader_Macros /*macros*/, nullptr /*include*/, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelBC, &errorPixelCode);

	// Создаём объекты шейдеров на GPU из скомпилированного байт-кода
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	device->CreateVertexShader(
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		nullptr, &vertexShader);

	device->CreatePixelShader(
		pixelBC->GetBufferPointer(),
		pixelBC->GetBufferSize(),
		nullptr, &pixelShader);


// �== INPUT LAYOUT — описание формата вершинных данных ==ค

	// Сообщаем D3D, как интерпретировать данные в вершинном буфере:
	//   - POSITION: 4 float (x, y, z, w) — позиция вершины
	//   - COLOR:    4 float (r, g, b, a) — цвет вершины
	D3D11_INPUT_ELEMENT_DESC inputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,	// Смещение вычисляется автоматически после предыдущего элемента
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};

	// Создаём Input Layout — связывает описание формата с байт-кодом вершинного шейдера
	ID3D11InputLayout* layout;
	device->CreateInputLayout(
		inputElements,
		2,
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		&layout);


// �== ВЕРШИННЫЕ ДАННЫЕ ==ค

	// Массив из 4 вершин; каждая вершина = позиция (XMFLOAT4) + цвет (XMFLOAT4).
	// Вместе это 8 элементов XMFLOAT4 = 4 пары (позиция, цвет).
	// Формирует прямоугольник (квад) из двух треугольников.
	DirectX::XMFLOAT4 points[8] = {
		DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	// Правый верхний — красный
		DirectX::XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.0f),	DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),	// Левый нижний — синий
		DirectX::XMFLOAT4(0.5f, -0.5f, 0.5f, 1.0f),	DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),	// Правый нижний — зелёный
		DirectX::XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),	// Левый верхний — белый
	};


	// Описание вершинного буфера (Vertex Buffer) — область памяти GPU для хранения вершин
	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;		// GPU имеет полный доступ на чтение/запись
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;				// CPU не обращается к этому буферу
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(DirectX::XMFLOAT4) * std::size(points);	// Общий размер буфера в байтах

	// Начальные данные для заполнения буфера при создании
	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = points;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	ID3D11Buffer* vb;
	device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);


// �== ИНДЕКСНЫЙ БУФЕР ==ค

	// Индексы определяют, какие вершины образуют треугольники.
	// Два треугольника: (0,1,2) и (1,0,3) — вместе формируют прямоугольник.
	int indeces[] = { 0,1,2};
	D3D11_BUFFER_DESC indexBufDesc = {};
	indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.CPUAccessFlags = 0;
	indexBufDesc.MiscFlags = 0;
	indexBufDesc.StructureByteStride = 0;
	indexBufDesc.ByteWidth = sizeof(int) * std::size(indeces);

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indeces;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	ID3D11Buffer* ib;
	device->CreateBuffer(&indexBufDesc, &indexData, &ib);

	// Шаг (stride) = 32 байта на вершину: 16 байт позиция (float4) + 16 байт цвет (float4)
	UINT strides[] = { 32 };
	UINT offsets[] = { 0 };


// �== СОСТОЯНИЕ РАСТЕРИЗАТОРА ==ค

	// Растеризатор преобразует треугольники из вершин в пиксели.
	// CullMode = NONE — не отбрасываем ни переднюю, ни заднюю грань (рисуем обе стороны).
	// FillMode = SOLID — заполняем треугольники целиком (не wireframe).
	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	ID3D11RasterizerState* rastState;
	res = device->CreateRasterizerState(&rastDesc, &rastState);

	context->RSSetState(rastState);


// �== ИГРОВОЙ ЦИКЛ ==ค

	// Переменные для подсчёта FPS
	std::chrono::time_point<std::chrono::steady_clock> PrevTime = std::chrono::steady_clock::now();
	float totalTime = 0;
	unsigned int frameCount = 0;


	MSG msg = {};
	bool isExitRequested = false;
	while (!isExitRequested) {
		// Обрабатываем все накопившиеся сообщения Windows (ввод, закрытие, перерисовка и т.д.)
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Если получено WM_QUIT (от PostQuitMessage) — выходим из цикла
		if (msg.message == WM_QUIT) {
			isExitRequested = true;
		}

		// Сбрасываем все привязки конвейера в состояние по умолчанию.
		// Это нужно, потому что предыдущий кадр мог оставить привязки,
		// которые конфликтуют с текущими настройками (например, текстура привязана и как SRV, и как RTV)
		context->ClearState();

		// Заново устанавливаем состояние растеризатора после ClearState
		context->RSSetState(rastState);

		// Настраиваем viewport — определяет, в какую область экрана D3D будет рисовать.
		// Здесь: весь экран, глубина от 0 до 1.
		D3D11_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(screenWidth);
		viewport.Height = static_cast<float>(screenHeight);
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1.0f;

		context->RSSetViewports(1, &viewport);

		// Настраиваем Input Assembler (IA) — первую стадию графического конвейера:
		//   - Формат вершинных данных (layout)
		//   - Топология примитивов (треугольники)
		//   - Индексный буфер (какие вершины образуют треугольники)
		//   - Вершинный буфер (сами данные вершин)
		context->IASetInputLayout(layout);
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
		context->IASetVertexBuffers(0, 1, &vb, strides, offsets);

		// Устанавливаем шейдеры для текущего кадра
		context->VSSetShader(vertexShader, nullptr, 0);
		context->PSSetShader(pixelShader, nullptr, 0);


		// Замеряем время между кадрами (delta time) для подсчёта FPS
		auto	curTime = std::chrono::steady_clock::now();
		float	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
		PrevTime = curTime;

		totalTime += deltaTime;
		frameCount++;

		// Каждую секунду обновляем заголовок окна, показывая текущий FPS
		if (totalTime > 1.0f) {
			float fps = frameCount / totalTime;

			totalTime -= 1.0f;

			WCHAR text[256];
			swprintf_s(text, TEXT("FPS: %f"), fps);
			SetWindowText(hWnd, text);

			frameCount = 0;
		}

		// Привязываем Render Target View — D3D будет рисовать в задний буфер swap chain
		context->OMSetRenderTargets(1, &rtv, nullptr);

		// Очищаем задний буфер. Красный канал = totalTime (плавно меняется от 0 до 1),
		// поэтому фон будет периодически пульсировать от тёмного к красному.
		float color[] = { totalTime, 0.1f, 0.1f, 1.0f };
		context->ClearRenderTargetView(rtv, color);

		// Рисуем 6 индексов (2 треугольника = 1 прямоугольник)
		context->DrawIndexed(6, 0, 0);

		// Отвязываем render target перед показом кадра
		context->OMSetRenderTargets(0, nullptr, nullptr);

		// Показываем кадр на экране. Первый параметр = 1 означает VSync (ждём вертикальной синхронизации)
		swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
	}

    std::cout << "Hello World!\n";
}
