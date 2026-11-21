USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListDragonJewel]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-06-11
description : ?? ???? ?????.(????)
result set : ParentItemSerial/bigint, DragonJewelLocationIndex/tinyint, ItemID/int, ItemPotential/tinyint, SequenceNumber/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListDragonJewel]
	  @vchItemSerial varchar(max) --// ??? ??? ??? (, ???)		
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @nvcSelectStmt varchar(max);

DECLARE @tblParentItemSerials table (
	ItemSerial bigint NOT NULL,
	SequenceNumber int NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [DragonJewels] ???? SELECT???.*/
	IF @vchItemSerial IS NOT NULL 	
	BEGIN
		INSERT @tblParentItemSerials (ItemSerial, SequenceNumber)
		SELECT CAST (string AS bigint), CAST(seq AS int)
		FROM dbo.TF_Split(@vchItemSerial, ',');
		
		SELECT DJ.ParentItemSerial, DJ.DragonJewelLocationIndex, DJ.ItemID, DJ.ItemOption, SI.SequenceNumber
		FROM dbo.DragonJewels DJ WITH (NOLOCK, INDEX(IX_UNI_DragonJewels_ParentItemSerial_DragonJewelLocationIndex), FORCESEEK)
			 INNER JOIN @tblParentItemSerials SI ON DJ.ParentItemSerial = SI.ItemSerial;			 

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103390;
			GOTO ErrorHandler;
		END
	END
END TRY
BEGIN CATCH
	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO
