USE [dnmembership]
GO
/****** Object:  Table [dbo].[MaintenanceFlag]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[MaintenanceFlag]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[MaintenanceFlag](
	[MaintenanceFlag] [bit] NULL
) ON [DataFG1]
END
GO
