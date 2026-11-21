USE [dnmembership]
GO
/****** Object:  Table [dbo].[ReleasedIPs]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[ReleasedIPs]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[ReleasedIPs](
	[ReleasedIPID] [int] IDENTITY(1,1) NOT NULL,
	[StartIP] [int] NOT NULL,
	[EndIP] [int] NOT NULL,
	[BlockedDate] [smalldatetime] NOT NULL,
	[BlockedDescription] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[ReleasedDate] [smalldatetime] NOT NULL
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NC_ReleasedIPs_StartIP_EndIP]    Script Date: 11/14/2026 8:51:04 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[ReleasedIPs]') AND name = N'IX_NC_ReleasedIPs_StartIP_EndIP')
CREATE CLUSTERED INDEX [IX_NC_ReleasedIPs_StartIP_EndIP] ON [dbo].[ReleasedIPs]
(
	[StartIP] ASC,
	[EndIP] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
