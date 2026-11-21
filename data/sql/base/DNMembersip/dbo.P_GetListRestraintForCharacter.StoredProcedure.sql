USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListRestraintForCharacter]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListRestraintForCharacter]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListRestraintForCharacter] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-05
description : ??? ?? ??? ?????.
result set : RestraintTypeCode/tinyint, RestraintReasonID/int, RestraintMessage/nvarchar, StartDate/smalldatetime, EndDate/smalldatetime
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101112 = ?? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_GetListRestraintForCharacter]
	  @intCharacterID int --// ?? ?? ???
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
	SELECT RestraintTypeCode, RestraintReasonID, RestraintMessage, StartDate, EndDate
	FROM dbo.Restraints WITH (NOLOCK)
	WHERE CharacterID = @intCharacterID AND CAST(GETDATE() AS smalldatetime) BETWEEN StartDate AND EndDate AND CancelFlag = CAST(0 AS bit);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101112;
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
