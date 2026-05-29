UIManager = {
    widgets = {},
    isInitialized = false
}

UI.Callbacks = UI.Callbacks or {}

function UIManager:RegisterWidget(widgetName, widgetTable)
    if not widgetTable.Init then
        print("[UI Error] Failed to register '" .. widgetName .. "': Missing lifecycle hooks.")
        return
    end

    self.widgets[widgetName] = widgetTable
    print("[UI] Successfully registered widget: " .. widgetName)
end

--Adds a click to an entity
function UIManager.SetOnClick(entityId, callback)
    UI.Callbacks[entityId] = callback
end

--Removes the enttiy from the callback list
function UIManager.ClearOnClik(entityId)
    UI.Callbacks[entityId] = nil
end

function UI.DispatchClick(entityId)
    local cb = UI.Callbacks[entityId]
    if cb then
        cb(entityId)
    end
end

function UIManager:Init()
    print("[UI] Initializing Master Router...")

    local filesToLoad = {
        "test.lua"
    }

    for _, file in ipairs(filesToLoad) do
        local success, err = pcall(UI.FileSystem.ExecuteScript, file)
        if not success then
            print("[UI Error] VFS execution failed for " .. file .. ": " .. tostring(err))
        end
    end

    for name, widget in pairs(self.widgets) do
        local success, err = pcall(widget.Init, widget)
        if not success then
            print("[UI Error] Init failed for " .. name .. ": " .. tostring(err))
        end
    end

    self.isInitialized = true
end

function UIManager:Deinit()
    print("[UI] Deinitializing Master Router...")

    for name, widget in pairs(self.widgets) do
        local success, err = pcall(widget.Deinit, widget)
        if not success then
            print("[UI Error] DeInit failed for " .. name .. ": " .. tostring(err))
        end
    end

    self.isInitialized = false
end
