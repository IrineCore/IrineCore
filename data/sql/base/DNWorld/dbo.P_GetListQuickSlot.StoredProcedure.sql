USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListQuickSlot]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-13
description : ? ?? ??? ?????.
result set : QuickSlotIndex/tinyint, ObjectTypeCode/tinyint, ObjectValue/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103107 = ? ??? ???? ????.
history :
3 = ObjectValue ??? ??? ??? int -> bigint? ?? at 2011-02-15
**/

CREATE PROCEDURE [dbo].[P_GetListQuickSlot]
	  @inbCharacterID bigint --// ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [QuickSlots] ???? SELECT???.*/
	SELECT QuickSlotIndex, ObjectTypeCode, ObjectValue
	FROM dbo.QuickSlots WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;

	/**_# ? ??? ???? ??? 103107? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103107;
		GOTO ErrorHandler;
	END
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
