USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPLadderCuCountLogs]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPLadderCuCountLogs](
	[BasisDate] [datetime2](0) NOT NULL,
	[PVPLadderCode] [tinyint] NOT NULL,
	[CUCount] [int] NOT NULL,
 CONSTRAINT [PK_PVPLadderCuCountLogs] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[PVPLadderCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
