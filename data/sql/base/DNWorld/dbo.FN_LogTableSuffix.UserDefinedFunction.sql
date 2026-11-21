USE [dnworld]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_LogTableSuffix]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
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
	RETURN DATEDIFF(DAY, '2000-01-01', @dt2BasisDate) % 3;
END
GO
