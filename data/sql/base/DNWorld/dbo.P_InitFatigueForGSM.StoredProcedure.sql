USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_InitFatigueForGSM]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-11
description : ???? ?? ??????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ?? ?? ?? (?? ??) at 2010-06-11
4 = ??? ?? ?? at 2010-07-20
**/

CREATE PROCEDURE [dbo].[P_InitFatigueForGSM]
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbCharacterID bigint = 0;
DECLARE @tblTemp table (CharacterID bigint NOT NULL PRIMARY KEY);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	WHILE 1 = 1
	BEGIN
		DELETE @tblTemp;

		WITH C1 AS (
			SELECT TOP (100) CharacterID
			FROM dbo.V_Characters WITH (NOLOCK)
			WHERE CharacterID > @inbCharacterID
				AND LastFatigueDate >= DATEADD(HOUR, 4, CAST(CAST(GETDATE() AS date) AS datetime2(0))) AND DeleteFlag = CAST(0 AS bit)
			ORDER BY CharacterID
		)
		UPDATE dbo.CharacterStatus
		SET LastFatigueDate = DATEADD(HOUR, 3, CAST(CAST(GETDATE() AS date) AS datetime2(0)))
		OUTPUT deleted.CharacterID INTO @tblTemp (CharacterID)
		FROM dbo.CharacterStatus CS
			INNER JOIN C1 ON CS.CharacterID = C1.CharacterID;

		IF @@ROWCOUNT = 0
			BREAK;

		SELECT @inbCharacterID = MAX(CharacterID) FROM @tblTemp;
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
