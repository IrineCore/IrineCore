USE [dnstaging]
GO
/****** Object:  DdlTrigger [DDL_TRG_SQLSafe]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TRIGGER [DDL_TRG_SQLSafe] ON DATABASE
FOR DDL_TABLE_EVENTS, DDL_VIEW_EVENTS, DDL_INDEX_EVENTS, DDL_FUNCTION_EVENTS, DDL_PROCEDURE_EVENTS, DDL_TRIGGER_EVENTS
AS
SET NOCOUNT ON;

DECLARE
    @xmlData xml,
    @nvcEventType nvarchar(128),
    @nvcObjectName nvarchar(128),
    @nvcStmt nvarchar(4000);

IF CURRENT_USER = N'SPExecutor'
    RETURN;

SET @xmlData = EVENTDATA();
SET @nvcEventType = @xmlData.value('(/EVENT_INSTANCE/EventType)[1]', 'nvarchar(128)');
SET @nvcObjectName = @xmlData.value('(EVENT_INSTANCE/ObjectName)[1]', 'nvarchar(128)');

IF @nvcEventType IN (N'CREATE_PROCEDURE') AND @nvcObjectName NOT LIKE N'sp/_%' ESCAPE(N'/')
BEGIN
    SET @nvcStmt = N'GRANT EXECUTE ON OBJECT::[' + @nvcObjectName + N'] TO [AppServer];'
    EXEC (@nvcStmt);
END;
GO
ENABLE TRIGGER [DDL_TRG_SQLSafe] ON DATABASE
GO
