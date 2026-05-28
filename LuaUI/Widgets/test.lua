local Test = {}

function Test:Init()

    self.bg = UI.CreateImage("Assets/Textures/cover.png")

     UI.SetLayout(self.bg, {
    position = {100.0, 100.0},
       size = {100.0, 100.0},
       anchorMin = {0.0, 0.0},
       anchorMax = {1.0, 1.0},
       pivot = {0.5, 0.5},
       localZ = 0
    })

     self.box = UI.CreateVBox({
        offset = {0.0, 0.0, 0.0, 0.0},
        spacing = 20.0,
        childStart = ChildStart.Middle,
        controlHSize = true,
        controlVSize = true,
    })
    UI.SetParent(self.box, self.bg)
    

      UI.SetLayout(self.box, {
    position = {100.0, 100.0},
       size = {100.0, 100.0},
       anchorMin = {0.0, 0.0},
       anchorMax = {1.0, 1.0},
       pivot = {0.5, 0.5},
       localZ = 0
    })

    self.elements = {}
    for i = 1, 5 do
        self.elements[i] = UI.CreateImage()
        local text = UI.CreateText(
        {
            fontName = "CauseFont.casset",
            text = "BUTTON",
            color = { 0.0, 0.0, 0.0, 1.0},
            fontSize = 60.0
        })

        UI.AddNineSlice(self.elements[i], 
        {
            left = 30.0,
            right = 30.0,
            top = 30.0,
            bottom = 30.0
        })

        UI.SetLayout(text, {
            position = {0.0, 0.0},
            size = {0.0, 0.0},
            anchorMin = {0.0, 0.0},
            anchorMax = {1.0, 1.0},
            pivot = {0.5, 0.5},
            localZ = 0
        })

        UI.SetParent(text, self.elements[i])

        UI.SetTextStyle(text,
        {
          charSpacing = 0.0,
          lineSpacing = 0.0,
          horizontal = TextHAlign.Center,
          vertical = TextVAlign.Middle,
          wrapText = true
        })

        UI.SetParent(self.elements[i], self.box)
    end
end

function Test:Deinit()
    UI.DestroyEntity(self.bg);
end

UIManager:RegisterWidget("Test", Test)