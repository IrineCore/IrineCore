USE [dnmembership]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_LogTableSuffix]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[FN_LogTableSuffix]') AND type in (N'FN', N'IF', N'TF', N'FS', N'FT'))
BEGIN
execute dbo.sp_executesql @statement = N'/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-06
description : ??? ???? ?? ???? ??? (0 or 1 or 2)? ?????.
**/

CREATE FUNCTION [dbo].[FN_LogTableSuffix] (
	@dt2BasisDate datetime2(0)
)
RETURNS int
AS
BEGIN
	RETURN DATEDIFF(DAY, ''2000-01-01'', @dt2BasisDate) % 3;
END
' 
END
GO
