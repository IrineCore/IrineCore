USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCharacterStarterItem]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-05-08
description : ??? ??? ??? ???? ?????.
resule set : CharacterStarterItemID/int, ItemID/int, ItemLocationCode/tinyint, ItemLocationIndex/tinyint, ProvideStartDate/smalldatetime, ProvideEndDate/smalldatetime, RegisterDate/smalldatetime
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListCharacterStarterItem]
	  @intItemID int = NULL--// ?? ???ID	
	, @sdtProvideStartDate smalldatetime = NULL --// ?? ?? ??
	, @sdtProvideEndDate smalldatetime = NULL --// ?? ?? ??	
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
	/**_# [CharacterStarterItems] ???? SELECT???.*/
	SELECT CharacterStarterItemID, ItemID, ItemLocationCode, ItemLocationIndex, ProvideStartDate, ProvideEndDate, RegisterDate
	FROM dbo.CharacterStarterItems WITH (NOLOCK)
	WHERE ItemID = COALESCE(@intItemID, ItemID) 	
		  AND (ProvideStartDate IS NULL AND ProvideEndDate IS NULL) 
		  OR (ProvideStartDate <= ISNULL(@sdtProvideEndDate, '2079-01-01') AND ProvideEndDate >= ISNULL(@sdtProvideStartDate, '1900-01-01'))
		  OR (ProvideStartDate IS NULL AND (ISNULL(@sdtProvideStartDate, '1900-01-01') <= ProvideEndDate))
		  OR (ProvideEndDate IS NULL AND (ISNULL(@sdtProvideEndDate, '2079-01-01') >= ProvideStartDate));

END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
