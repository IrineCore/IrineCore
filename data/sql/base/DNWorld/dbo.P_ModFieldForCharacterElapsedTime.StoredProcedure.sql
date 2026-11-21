USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModFieldForCharacterElapsedTime]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-05-27
description : ?? ????? ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
**/

CREATE PROCEDURE [dbo].[P_ModFieldForCharacterElapsedTime]
	  @inbCharacterID int --// ??? ID
	, @vchFieldIndexes varchar(max) --// ,? ?????? ??? ??? ID ??? (?? 500?)
	, @vchElapsedTimes varchar(max) --// ,? ?????? ??? ?? ?? ??? (?? 500?)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblTemp table (FieldIndex smallint NOT NULL PRIMARY KEY, ElapsedTime int NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblTemp (FieldIndex, ElapsedTime)
	SELECT CAST(A.string AS smallint), CAST(B.string AS int)
	FROM dbo.TF_Split(@vchFieldIndexes, ',') A
		INNER JOIN dbo.TF_Split(@vchElapsedTimes, ',') B ON A.seq = B.seq
	WHERE A.string > '' AND B.string > ''
	OPTION (KEEPFIXED PLAN, MAXDOP 1);

	/**_# [Fields] ???? UPDATE???.*/
	UPDATE dbo.FieldsForCharacter
	SET ElapsedTime = T.ElapsedTime, LastUpdateDate = GETDATE()
	FROM @tblTemp T
		INNER JOIN dbo.FieldsForCharacter FL WITH (INDEX(PK_FieldsForCharacter), FORCESEEK) ON FL.FieldIndex = T.FieldIndex
	WHERE FL.CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);
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
