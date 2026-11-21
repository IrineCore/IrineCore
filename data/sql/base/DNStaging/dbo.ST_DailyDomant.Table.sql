USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_DailyDomant]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_DailyDomant](
	[BasisDate] [date] NOT NULL,
	[RegisterCode] [tinyint] NOT NULL,
	[D7LeaveCount] [int] NULL,
	[D15LeaveCount] [int] NULL,
	[D30LeaveCount] [int] NULL,
	[D7ReviveCount] [int] NULL,
	[D15ReviveCount] [int] NULL,
	[D30ReviveCount] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_ST_DailyDomant_BasisDate]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ST_DailyDomant_BasisDate] ON [dbo].[ST_DailyDomant]
(
	[BasisDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
