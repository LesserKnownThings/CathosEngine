UIManager = {
    widgets = {},
    isInitialized = false
}

UI.Callbacks = UI.Callbacks or {}

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
        cb()
    end
end

function UIManager:Init()
    print("[UI] Initializing Master Router...")

    for _, filePath in ipairs(UI.VFS.GetFilesRecursive("Widgets")) do
        local widget = dofile(filePath)
        pcall(widget.Init, widget)
        table.insert(UIManager.widgets, widget)
    end

    self.isInitialized = true
end

function UIManager:Deinit()
    print("[UI] Deinitializing Master Router...")

    for _, widget in ipairs(self.widgets) do
        pcall(widget.Deinit, widget)
    end

    self.isInitialized = false
end
