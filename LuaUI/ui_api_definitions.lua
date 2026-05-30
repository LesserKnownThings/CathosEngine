--Global used for all UI actions
UI = {}

--Global virtual file server, used to get stuff relative to the LuaUI folder
UI.VFS = {}

--Creates an image
--@param texturePath string or nill, use to assing a specific texture to the image
--@return integer the newly created image entity id
function UI.CreateImage(texturePath) end

--Creates an emtpy container
--@return integer the newly created entity id
function UI.CreateWidget() end

--Creates a text box
--@param textData a table used by the engine to create the text
--@return integer the newly created text box entity id
function UI.CreateText(textData) end

--Example of CreateText
-- UI.CreateText(
--         {
--             fontPath = "Assets/Fonts/CauseFont.casset",
--             text = "Hello from LUA!!",
--             color = { 0.0, 0.0, 0.0, 1.0},
--             fontSize = 60.0
--         }
--     )

--Destroys any entity using the entity id
--@param entity integer entity
function UI.DestroyEntity(entity) end

--Sets the visbility of the entity
--@param entity id
--@param mode an enum VisiblityMode with the type of visibility for the widegt
function UI.SetVisibility(entity, visbilityMode) end

--Sets the parent of an entity
--@param child the entity id that will be the child
--@param parent the entity id that will be the parent
function UI.SetParent(child, parent) end

--Set layout for a widget
--The layout is the actuall transform of the widget
--@param entity id
--@param data a table of params to set
function UI.SetLayout(entity, data) end

--Set layout example
-- UI.SetLayout(self.bg,
--     {
--       position = {0.0, 0.0},
--       size = {300.0, 300.0},
--       anchorMin = {0.5, 0.5},
--       anchorMax = {0.5, 0.5},
--       pivot = {0.5, 0.5},
--       localZ = 0,
--     })

--Sets the style of the text
--@param entity the entity id for the text to change
--@param style a table for with the text style
function UI.SetTextStyle(entity, style) end

--SetTextStyle example
-- UI.SetTextStyle(self.widget,
--         {
--           charSpacing = 0.0,
--           lineSpacing = 0.0,
--           horizontal = TextHAlign.Center,
--           vertical = TextVAlign.Middle,
--           wrapText = true
--         }
--     )

--Adds a nine slice component to an image, technically you can add it to other entities, but it doesn't make sense
--@param entity the entity id of the image
--@param table of 4 floats for each edge of the image
function UI.AddNineSlice(entity, edges) end

--Example of nine slice
--  UI.AddNineSlice(self.bg,
--     {
--         left = 60.0,
--         right = 60.0,
--         top = 60.0,
--         bottom = 60.0,
--     })

--Creates a horizontal box
--@param data a table with the table for the hbox
--@return entity id
function UI.CreateHBox(data) end

--Example of CreateHBox
-- UI.CreateHBox({
--         offset = {0.0, 0.0, 0.0, 0.0},
--         spacing = 0.0,
--         childStart = ChildStart.Start,
--         controlHSize = false,
--         controlVSize = false,
--     })

--Creates a vertical box
--@param data a table with the table for the vbox
--@return entity id
function UI.CreateVBox(data) end

--Example of CreateVBox
-- UI.CreateVBox({
--         offset = {0.0, 0.0, 0.0, 0.0},
--         spacing = 0.0,
--         childStart = ChildStart.Start,
--         controlHSize = false,
--         controlVSize = false,
--     })

--Creates a hierarchy and returns the parent
--@param buttonData is a table that holds the required button params
--@param textData is a table that holds the required text params
--@return entityId of the parent
function UI.CreateButton(buttonData, textData) end

--Example of CreateButton
--This is an exmaple with text
-- UI.CreateButton(
--             {
--                 texturePath = nil, --Will load the default white texture
--                 normal = {1.0, 1.0, 1.0, 1.0},
--                 hover = {1.0, 1.0, 0.0, 1.0},
--                 press = {0.0, 1.0, 1.0, 1.0}
--             },
--             {
--                 fontPath = "Assets/Fonts/CauseFont.casset",
--                 text = "Button",
--                 fontSize = 35.0,
--                 textColor = {0.0, 0.0, 0.0, 1.0},
--                 horizontal = TextHAlign.Center,
--                 vertical = TextVAlign.Middle
--             }
--         )

--This is an example without text
-- UI.CreateButton(
--             {
--                 texturePath = nil, --Will load the default white texture
--                 normal = {1.0, 1.0, 1.0, 1.0},
--                 hover = {1.0, 1.0, 0.0, 1.0},
--                 press = {0.0, 1.0, 1.0, 1.0}
--             }
--         )

--The send message has 2 variants, you can send a message with the entity or without so the entity is optional
--@param entity optional param for the entity Id, some messages don't require an etity to work
--@param msg a table containing the id of the function and the value
--The list of UI messages can be found in the Message.h under UIMessages or in the doc
function UI.SendUIMessage(entity, msg) end
