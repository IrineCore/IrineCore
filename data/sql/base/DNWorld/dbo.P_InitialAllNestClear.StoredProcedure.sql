USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_InitialAllNestClear]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-07-16
description : ?? ??? ??? ??? ??????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ??? ?? ??? ??? ?? ?? by ??? at 2012-12-05
4 = 2? ?? ?? by ??? at 2012-12-05
**/

CREATE PROCEDURE [dbo].[P_InitialAllNestClear]
	  @vchMapID varchar(max) = NULL --// ,? ?????? ?ID ???. (NULL ??? ?? ?? ??????)
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
	
	SELECT CAST(string AS int) AS MapID
	INTO #MapIDs
    FROM dbo.TF_Split(@vchMapID, ',');

	/**_# [NestClear] ???? UPDATE???.*/
	IF @@ROWCOUNT = 0
	BEGIN
		CREATE TABLE #Temp (MapID int NOT NULL);	
		WHILE 1 = 1
		BEGIN
			TRUNCATE TABLE #Temp;

			INSERT #Temp (MapID)
			SELECT TOP (500) MapID
			FROM dbo.NestClear WITH (NOLOCK)
			WHERE ClearCount > 0
			ORDER BY CharacterID;

			IF @@ROWCOUNT = 0
				BREAK;

			UPDATE dbo.NestClear
			SET ClearCount = 0, PCRoomClearCount = 0
			FROM dbo.NestClear NC
				INNER JOIN #Temp T ON NC.MapID = T.MapID;
		END	
		DROP TABLE #Temp;			
	END
	ELSE	
	BEGIN		
		UPDATE dbo.NestClear
		SET ClearCount = 0, PCRoomClearCount = 0
		FROM dbo.NestClear NC
			INNER JOIN #MapIDs M ON NC.MapID = M.MapID;		
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
