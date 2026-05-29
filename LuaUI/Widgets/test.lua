local Test = {}

local function Play()
    print("Play!")
end

local function Options()
    print("Options!")
end

local function Quit()
    UI.SendUIMessage({
        id = "close_game",
        value = nil,
    })
end

function Test:Init()
    self.bg = UI.CreateImage("Assets/Textures/cover.png")

    UI.SetLayout(self.bg, {
        position = { 0.0, 0.0 },
        size = { 600.0, 800.0 },
        anchorMin = { 0.5, 0.5 },
        anchorMax = { 0.5, 0.5 },
        pivot = { 0.5, 0.5 },
        localZ = 0
    })
    UI.AddNineSlice(self.bg,
        {
            left = 30.0,
            right = 30.0,
            top = 30.0,
            bottom = 30.0
        })

    self.box = UI.CreateVBox({
        offset = { 0.0, 0.0, 0.0, 0.0 },
        spacing = 20.0,
        childStart = ChildStart.Middle,
        controlHSize = true,
        controlVSize = true,
    })
    UI.SetParent(self.box, self.bg)


    UI.SetLayout(self.box, {
        position = { 100.0, 100.0 },
        size = { 100.0, 100.0 },
        anchorMin = { 0.0, 0.0 },
        anchorMax = { 1.0, 1.0 },
        pivot = { 0.5, 0.5 },
        localZ = 0
    })

    self.elements = {}

    self.elements[0] = UI.CreateButton(
        {
            texturePath = nil, --Will load the default white texture
            normal = Color.ui_button,
            hover = Color.ui_hover,
            press = Color.ui_pressed
        }
    )

    UIManager.SetOnClick(self.elements[0], Play)
    UI.SetParent(self.elements[0], self.box)

    self.elements[1] = UI.CreateButton(
        {
            texturePath = nil, --Will load the default white texture
            normal = Color.ui_button,
            hover = Color.ui_hover,
            press = Color.ui_pressed
        },
        {
            fontPath = "Assets/Fonts/CauseFont.casset",
            text = "Options",
            fontSize = 35.0,
            textColor = Color.black,
            horizontal = TextHAlign.Center,
            vertical = TextVAlign.Middle
        }
    )

    UIManager.SetOnClick(self.elements[1], function() UI.SetVisibility(self.bg, VisibilityMode.Collapsed) end)
    UI.SetParent(self.elements[1], self.box)

    self.elements[2] = UI.CreateButton(
        {
            texturePath = nil, --Will load the default white texture
            normal = Color.ui_button,
            hover = Color.ui_hover,
            press = Color.ui_pressed
        },
        {
            fontPath = "Assets/Fonts/CauseFont.casset",
            text = "Quit",
            fontSize = 35.0,
            textColor = Color.black,
            horizontal = TextHAlign.Center,
            vertical = TextVAlign.Middle
        }
    )

    UIManager.SetOnClick(self.elements[2], Quit)
    UI.SetParent(self.elements[2], self.box)
end

function Test:Deinit()
    for value in ipairs(self.elements) do
        UIManager.ClearOnClik(value);
    end

    UI.DestroyEntity(self.bg);
end

UIManager:RegisterWidget("Test", Test)
