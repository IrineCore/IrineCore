USE [dnworld]
GO
/****** Object:  Table [dbo].[ItemCountMonitoringTargetBuffer]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemCountMonitoringTargetBuffer](
	[ItemID] [int] NOT NULL,
	[ItemLevel] [tinyint] NOT NULL,
 CONSTRAINT [PK_ItemCountMonitoringTargetBuffer] PRIMARY KEY CLUSTERED 
(
	[ItemID] ASC,
	[ItemLevel] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
