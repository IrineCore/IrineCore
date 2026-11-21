USE [dnstaging]
GO
/****** Object:  Table [dbo].[PointLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PointLogs](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[PointCode] [tinyint] NOT NULL,
	[PointBefore] [int] NOT NULL,
	[PointAfter] [int] NOT NULL,
	[MapID] [int] NOT NULL,
	[IP] [int] NOT NULL
) ON [PRIMARY]
GO
