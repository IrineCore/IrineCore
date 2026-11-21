USE [dnworld]
GO
/****** Object:  Table [dbo].[PointLogs0]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PointLogs0](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[PointCode] [tinyint] NOT NULL,
	[PointBefore] [int] NOT NULL,
	[PointAfter] [int] NOT NULL,
	[MapID] [int] NOT NULL,
	[IP] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_PointLogs0_LogDate]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_PointLogs0_LogDate] ON [dbo].[PointLogs0]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
